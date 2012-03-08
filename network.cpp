/*
 * network.cpp
 *
 * Copyright (C) 1999, Ben Swartzlander
 * This file is part of Q2BotCore.
 * For conditions of distribution and use,
 * see the accompanying README file.
 */

#include "util.h"
#include "network.h"

#include <errno.h>
#include <string.h>

#ifdef  WIN32
# include <windows.h>
# include <winsock.h>
#else
# include <netinet/in.h>
# include <netdb.h>
# include <pthread.h>
# include <sys/socket.h>
# include <unistd.h>
# define INVALID_SOCKET    -1
# define SOCKET_ERROR      -1
# define closesocket       close
#endif

#include <time.h>

typedef char uikey_t[32];
typedef char uivalue_t[32];

typedef struct console_message_s {
	char *s;
	struct console_message_s *next;
} console_message_t;

bool connected=false;
bool kill_console_thread=false;

#if WIN32
  SOCKET sock;
  sockaddr_in addr;

  HANDLE console_thread;
  DWORD console_thread_id;
  bool sendRLock=false;
  bool sendULock=false;
  bool consoleQueueLock=false;
#else
  int sock;
  struct sockaddr_in addr;

  pthread_t console_thread;
  pthread_mutex_t consoleQueueLock = PTHREAD_MUTEX_INITIALIZER;

  pthread_mutex_t sendRLock = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t sendULock = PTHREAD_MUTEX_INITIALIZER;
#endif

console_message_t *consoleQueue=NULL;

unsigned int cl_sequence=0;
unsigned int sv_sequence=0;
unsigned int cl_bit=0;
unsigned int sv_bit=0;
unsigned int last_reliable_seq=0;
unsigned int packet_loss=0;
extern unsigned int current_frame;
bool reliable_received=false;

unsigned int challenge_number=0;
unsigned short client_id=0;

ucmd_t updates[UPDATE_LIST_LENGTH+1];
int current_update=0;
int lastPingTime=0;
int runningPing=0;

uikey_t uiKeys[256];
uivalue_t uiValues[256];
int numUserInfos=0;

void sendReliablePacket(unsigned int,const char *,int);
void sendUnreliablePacket(unsigned int,const char *,int);
void sendUnorderedPacket(const char *,int);
void sendUserinfo(void);

unsigned int ip_address(const char *hostname) {
	int i;
	int ndots=0;
	char *(dots[4]);
	unsigned int addr=0;
  int a, b, c, d;
  
  if( 4 != sscanf(hostname, "%d.%d.%d.%d", &a, &b, &c, &d) ) {
    /* the address is probably a domain name, so bail out */
    return 0;
  }
  
  /* otherwise combine into an ipv4 addr */
  addr = (a&0xff) | ((b&0xff)<<8) | ((c&0xff)<<16) | ((d&0xff)<<24);

/*
	dots[ndots++]=hostname;
	for(i=0;hostname[i];i++) {
		if(hostname[i]=='.') {
			if(ndots==4) {
				return 0;
			}
			dots[ndots++]=&(hostname[i+1]);
			hostname[i]=0;
		}
	}
	if(ndots!=4) {
		return 0;
	}
	if(!sscanf(dots[0],"%d",&i)) return 0;
	addr|=(i&0xff);
	if(!sscanf(dots[1],"%d",&i)) return 0;
	addr|=((i&0xff)<<8);
	if(!sscanf(dots[2],"%d",&i)) return 0;
	addr|=((i&0xff)<<16);
	if(!sscanf(dots[3],"%d",&i)) return 0;
	addr|=((i&0xff)<<24);
*/
	return addr;
}

void netPreInit(void) {
	strcpy(uiKeys[0],"rate");
	strcpy(uiValues[0],"25000");
	strcpy(uiKeys[1],"msg");
	strcpy(uiValues[1],"1");
	strcpy(uiKeys[2],"fov");
	strcpy(uiValues[2],"90");
	strcpy(uiKeys[3],"skin");
	strcpy(uiValues[3],"male/flak");
	strcpy(uiKeys[4],"name");
	strcpy(uiValues[4],"Q2BotCore");
	strcpy(uiKeys[5],"hand");
	strcpy(uiValues[5],"2");
	numUserInfos=6;
}

bool netInit(const char *hostname, unsigned short port) {
	struct hostent *hp;
#ifdef WIN32
  WSADATA wsaData;

  print("--- Initializing Winsock ---\n");
  if(WSAStartup(0x0101,&wsaData)!=0) {
    print("Could not open socket library\n");
    return false;
  }
  print("Vendor: %s\n",wsaData.szDescription);
  print("Status: %s\n",wsaData.szSystemStatus);
  print("Winsock initialized\n");
#endif

	sock=socket(PF_INET,SOCK_DGRAM,0);
	if(sock==INVALID_SOCKET) {
		qbLog("Could not create socket");
#ifdef WIN32
    WSACleanup();
#endif    
		return false;
	}
	
	qbLog("--- Looking up server ---");
	addr.sin_addr.s_addr=ip_address(hostname);
	if(!addr.sin_addr.s_addr) {
		hp=gethostbyname(hostname);
		if(hp) {
			addr.sin_addr=*((struct in_addr *)hp->h_addr);
		} else {
			qbLog("Could not resolve host");
#ifdef WIN32      
      WSACleanup();
#endif
			return false;
		}
	}
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);

	if(connect(sock,(struct sockaddr *)&addr,sizeof(addr))==SOCKET_ERROR) {
		qbLog("Could not connect serverSock: %d",strerror(errno));
#ifdef WIN32    
    WSACleanup();
#endif    
		return false;
	}
	
	qbLog("Found server");
	connected=false;

	return true;
}

#if WIN32
  DWORD consoleThread(void* ptr) {
#else
  void* consoleThread(void *ptr) {
#endif  
	console_message_t *cm;

	while(!kill_console_thread) {
		
		int ms = _GetTickCount();
	
		LOCK(consoleQueueLock);	
		if(consoleQueue) {
			cm=consoleQueue;
			qbSynchConsole(cm->s);
			free(cm->s);
			consoleQueue=cm->next;
			free(cm);
		}
		UNLOCK(consoleQueueLock);
		
		int elapsed = qbTickCount() - ms;
		throttle( elapsed, QB_MIN_FRAMETIME );		
		
	}
	return 0;
}

bool netConnect(void) {
	int i;
	char userinfoString[1024];
	char buffer[2048];

	memset(updates,0,(UPDATE_LIST_LENGTH+1)*sizeof(ucmd_t));
	
	do {
		qbLog("Trying...");
		sendUnorderedPacket("getchallenge\x0a",13);
		Sleep(1000);
	} while(!challenge_number);
	
	srand(time(NULL));
	client_id=rand();
	userinfoString[0]='\0';
	for(i=0;i<numUserInfos;i++) {
		strcat(userinfoString,"\\");
		strcat(userinfoString,uiKeys[i]);
		strcat(userinfoString,"\\");
		strcat(userinfoString,uiValues[i]);
	}
	sprintf(buffer,"connect 34 %d %d \"%s\"\x0a",client_id,challenge_number,userinfoString);
	
	do {
		qbLog("Connecting...");
		sendUnorderedPacket(buffer,strlen(buffer));
		Sleep(1000);
	} while(!connected);

#if WIN32  
	console_thread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)consoleThread,NULL,0,&console_thread_id);
	if( NULL == console_thread ) {
#else
 	int ret=pthread_create(&console_thread, NULL, consoleThread, NULL);
	if(ret != 0) {
#endif
		qbLog("Could not create console thread");
		return false;
	}
/*TODO set pthread attribs to set a high priority*/
#ifdef WIN32
	SetThreadPriority(console_thread,THREAD_PRIORITY_TIME_CRITICAL);
#endif

	return true;
}

void netShutdown(void) {
/* shouldn't need this anymore with the additon of kill_console_thread */
/*	TerminateThread(console_thread,0); */
 	kill_console_thread=true;
	connected=false;
	closesocket(sock);
}

void netMovementOrder(void) {
	int i,j;
	char b[2048];
	unsigned char mask;
	int n,m;
	unsigned int seq1;
	
	seq1=++cl_sequence;
	i=0;
	*((unsigned short *)(b+i))=client_id;
	i+=2;
	b[i++]=0x02;
	b[i++]=0x00;
	*((unsigned long *)(b+i))=current_frame|packet_loss;
	i+=4;

	for(j=-3;j<0;j++) {
		n=(current_update+UPDATE_LIST_LENGTH+j+1)%UPDATE_LIST_LENGTH;
		m=(current_update+UPDATE_LIST_LENGTH+j)%UPDATE_LIST_LENGTH;
		if(j==-3) m=UPDATE_LIST_LENGTH;
		mask=0;
		if(updates[n].angles[0]!=updates[m].angles[0]) mask|=0x01; // pitch
		if(updates[n].angles[1]!=updates[m].angles[1]) mask|=0x02; // yaw
		if(updates[n].angles[2]!=updates[m].angles[2]) mask|=0x04; // roll
		if(updates[n].velocity[0]!=updates[m].velocity[0]) mask|=0x08;
		if(updates[n].velocity[1]!=updates[m].velocity[1]) mask|=0x10;
		if(updates[n].velocity[2]!=updates[m].velocity[2]) mask|=0x20;
		if(updates[n].buttons!=updates[m].buttons) mask|=0x40;
		if(updates[n].impulse!=updates[m].impulse) mask|=0x80;
		b[i++]=mask;
		if(mask&0x01) {
			*((short *)(b+i))=(short)(-updates[n].angles[0]*32768.0/PI);
			i+=2;
		}
		if(mask&0x02) {
			*((short *)(b+i))=(short)(updates[n].angles[1]*32768.0/PI);
			i+=2;
		}
		if(mask&0x04) {
			*((short *)(b+i))=(short)(updates[n].angles[2]*32768.0/PI);
			i+=2;
		}
		if(mask&0x08) {
			*((short *)(b+i))=(short)(updates[n].velocity[0]);
			i+=2;
		}
		if(mask&0x10) {
			*((short *)(b+i))=(short)(updates[n].velocity[1]);
			i+=2;
		}
		if(mask&0x20) {
			*((short *)(b+i))=(short)(updates[n].velocity[2]);
			i+=2;
		}
		if(mask&0x40) b[i++]=updates[n].buttons;
		if(mask&0x80) b[i++]=updates[n].impulse;
		b[i++]=updates[n].msec;
		b[i++]=updates[n].light;
	}
	
	current_update=(current_update+1)%UPDATE_LIST_LENGTH;

	b[3]=checksum((unsigned char *)(b+4),i-4,seq1);
	sendUnreliablePacket(seq1,b,i);
}

void qbUpdateUserinfo(const char *key, const char *value) {
	int i;
	
	for(i=0;i<numUserInfos;i++) {
		if(!strcmp(uiKeys[i],key)) {
			if(strcmp(uiValues[i],value)) {
				strcpy(uiValues[i],value);
				sendUserinfo();
				return;
			} else {
				return;
			}
		}
	}
	strcpy(uiKeys[numUserInfos],key);
	strcpy(uiValues[numUserInfos],value);
	numUserInfos++;
	sendUserinfo();
}

void sendUserinfo(void) {
	int length;
	char buffer[2048];
	char userinfoString[1024];
	int i;

	if(!connected) {
		return;
	}
	userinfoString[0]='\0';
	for(i=0;i<numUserInfos;i++) {
		strcat(userinfoString,"\\");
		strcat(userinfoString,uiKeys[i]);
		strcat(userinfoString,"\\");
		strcat(userinfoString,uiValues[i]);
	}
	length=strlen(userinfoString)+1;
	assert(length<=2045);
	*((unsigned short *)buffer)=client_id;
	buffer[2]=0x03;
	memcpy(buffer+3,userinfoString,length);
	sendReliablePacket(++cl_sequence,buffer,length+3);
}

void qbSynchConsole(const char *s) {
	int length;
	char buffer[2048];

	if(!connected) {
		return;
	}
	length=strlen(s)+1;
	assert(length<=2045);
	*((unsigned short *)buffer)=client_id;
	buffer[2]=0x04;
	memcpy(buffer+3,s,length);
/*	print("Send reliable: %s %08Xh\n",s,cl_sequence+1); */
	sendReliablePacket(++cl_sequence,buffer,length+3);
}

void qbAsynchConsole(const char *s) {
	console_message_t *cm,*i;

	cm=(console_message_t *)malloc(sizeof(console_message_t));
	cm->s=(char *)malloc(strlen(s)+1);
	strcpy(cm->s,s);
	cm->next=NULL;

 	LOCK(consoleQueueLock);
 
	int j=0;
	if(!consoleQueue) {
		consoleQueue=cm;
	} else {
		j++;
		for(i=consoleQueue;i->next!=NULL;i=i->next) { j++; }
		i->next=cm;
	}

 	UNLOCK(consoleQueueLock);
}

// General Purpose (Low Level) Network I/O

void sendUnorderedPacket(const char *data,int length) {
	char buffer[2048];

	assert(length<=2044);
 	assert(data != NULL);
	memcpy(buffer+4,data,length);
	*((unsigned int *)buffer)=0xffffffff;
	send(sock,buffer,length+4,0);
}

void sendReliablePacket(unsigned int seq1, const char *data,int length) {
	
	char buffer[2048];
	int loop;

	assert(length<=2040);
	assert(data != NULL);
	memcpy(buffer+8,data,length);

	LOCK(sendRLock);
  
	*((unsigned int *)buffer)=seq1|0x80000000;
	*((unsigned int *)(buffer+4))=sv_sequence|sv_bit;
	reliable_received=false;
	for(loop=0;!reliable_received;loop++) {
		if((loop%10)==0) {
			send(sock,buffer,length+8,0);
		}
		Sleep(50);
	}

 	UNLOCK(sendRLock);
}

void sendUnreliablePacket(unsigned int seq1,const char *data,int length) {
	char buffer[2048];

	assert(length<=2040);
	assert(data != NULL);
	memcpy(buffer+8,data,length);

	LOCK(sendULock);
      
	*((unsigned int *)buffer)=seq1;
	*((unsigned int *)(buffer+4))=sv_sequence|sv_bit;
	send(sock,buffer,length+8,0);

	UNLOCK(sendULock);
}

int recvPacket(char *data) {
	
	char buffer[2048];
	char b[256];
	int length;
	unsigned int seq;
	unsigned int temp;
	int pingTime;

	pingTime=_GetTickCount()-lastPingTime;
	if(pingTime>3000) {
		sprintf(b,"status");
		sendUnorderedPacket(b,7);
		lastPingTime=_GetTickCount();
	}

	length=recv(sock,buffer,2048,0)-8;
	if((length)==SOCKET_ERROR) {
		qbLog("Error Code: %d", strerror(errno));
		exit(1);
	}

	seq=*((unsigned int *)buffer);
	if(seq==0xffffffff) {
		buffer[length+8]=0;
		sscanf((char *)(buffer+4),"%s %d",b,&temp);
		if(!strcmp(b,"challenge")) {
			challenge_number=temp;
		} else if(!strcmp(b,"print")) {
			pingTime=_GetTickCount()-lastPingTime;
			runningPing=(2*runningPing+pingTime)/3;
		} else if(!strcmp(b,"client_connect")) {
			connected=true;
		}
		length=-1;
	} else {
		if(length) memcpy(data,buffer+8,length);
/*
		if((seq&0x7fffffff)==sv_sequence) {
			print("*** Recieved DUPLICATED packet");
		} else if((seq&0x7fffffff)<sv_sequence) {
			print("*** Recieved OUT OF ORDER packet");
		}
*/
		sv_sequence=seq;
		if(sv_sequence&0x80000000) {
			sv_sequence&=0x7fffffff;
//			print("Server send reliable: %08Xh\n",sv_sequence);
			if(sv_sequence>last_reliable_seq) {
				last_reliable_seq=sv_sequence;
				sv_bit^=0x80000000;
			}
		}
		seq=*((unsigned int *)(buffer+4));
		if((seq&0x80000000)!=cl_bit) {
//			print("Recieved ACK %08Xh\n",seq&0x7fffffff);
			cl_bit=seq&0x80000000;
			reliable_received=true;
		}
	}
	return length;
}
