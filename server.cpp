/*
 * server.cpp
 *
 * Copyright (C) 1999, Ben Swartzlander
 * This file is part of Q2BotCore.
 * For conditions of distribution and use,
 * see the accompanying README file.
 */

#include "util.h"
#include "network.h"

#include <fcntl.h>
#include <stdio.h>

#if WIN32
# include <io.h>
# include <windows.h>
#else
# include <unistd.h>
# include <pthread.h>
# define O_BINARY 0
#endif

#ifdef __linux__
# define VERSION_STRING "q2bot.so for Linux - v0.97 (2004/06/14)\n"
#elif __APPLE__
# define VERSION_STRING "q2bot.dylib for MacOSX - v0.97 (2006/06/20\n"
#elif WIN32
# define VERSION_STRING "q2bot.dll for Windows - v0.97 (2004/06/14)\n"
#endif

typedef char configstring_t[64];

typedef struct {
	vec3_t angles;
	vec3_t origin;
	vec3_t velocity;
	int gunindex;
	int stats[32];
} iplayer_t;

typedef struct {
	int timestamp;
	iplayer_t player;
	entity_t entities[1024];
} igamestate_t;

typedef struct {
	int timestamp;
	vec3_t origin;
} time_point_t;

#if WIN32
  HANDLE server_thread;
  DWORD  server_thread_id;  
	bool lock=false;
#else
  pthread_t server_thread;
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
#endif
  
bool kill_server_thread=false;
bool online=false;

unsigned int current_frame=0;
unsigned int last_frame=0xffffffff;
unsigned int delta_frame=0xffffffff;
extern unsigned int packet_loss;
extern int runningPing;

extern ucmd_t updates[];
extern int current_update;
int lastUpdateTime=0;

time_point_t data_points[1024];
igamestate_t states[17];
igamestate_t *cs=&(states[0]);
igamestate_t *ds=&(states[16]);
igamestate_t *spawn=&(states[16]);
int current_state=0;

configstring_t cstrings[1568];
int max_players;
int player_num;
int login_key;

int inventory[256];
int lastInvenTime=0;

bool recording=false;
int demo_handle;

int processServerPacket(unsigned char *b,int len) {
	int i=0;
	unsigned char type;
	char s[256];
	unsigned char dbuf[2048];
	int dlen=0;
	int i_start=0;
	int timestamp;

	timestamp=_GetTickCount();

	while(i<len) {
		type=b[i++];

/*
 *	0x01 - Muzzleflash
 */

		if(type==0x01) {
			i+=3;

/*
 *	0x02 - Monster Muzzleflash
 */

		} else if(type==0x02) {
			i+=3;

/*
 *	0x03 - Temporary Entity
 */

		} else if(type==0x03) {
			int entity_type=b[i++];
			switch(entity_type) {
				case 5:
				case 6:
				case 7:
				case 8:
				case 17:
				case 18:
				case 20:
				case 21:
				case 22:
				case 28:
				i+=6;
				break;

				case 0:
				case 1:
				case 2:
				case 4:
				case 9:
				case 12:
				case 13:
				case 14:
				case 27:
				i+=7;
				break;

				case 3:
				case 11:
				case 23:
				case 26:
				i+=12;
				break;

				case 10:
				i+=9;
				break;

				case 15:
				case 25:
				case 29:
				i+=9;
				break;

				case 16:
				case 19:
				i+=14;
				break;

				case 24:
				i+=20;
				break;

				default:
				qbLog("*** Unrecognized temp_entity type: %02Xh ***",entity_type);
				hexdump(b,len);
				return 0;
			}

/*
 *	0x04 - Layout
 */

		} else if(type==0x04) {
			while(b[i++]);

/*
 *	0x05 - Inventory
 */

		} else if(type==0x05) {
			int j;

			for(j=0;j<256;j++) {
				inventory[j]=*((short *)(b+i));
				i+=2;
			}
			lastInvenTime=_GetTickCount();

/*
 *	0x06 - NOOP
 */

		} else if(type==0x06) {
			;

/*
 *	0x07 - Disconnect
 */

		} else if(type==0x07) {
			return 1;

/*
 *	0x08 - Reconnect
 */

		} else if(type==0x08) {
			return 1;

/*
 *	0x09 - Sound
 */

		} else if(type==0x09) {
			unsigned int mask;

			mask=b[i++];
			i++;
			if(mask&0x01) i++;
			if(mask&0x02) i++;
			if(mask&0x10) i++;
			if(mask&0x08) i+=2;
			if(mask&0x04) i+=6;

/*
 *	0x0a - Print
 */

		} else if(type==0x0a) {
			int j=0;

			i++;
			while(b[i++]) {
				s[j++]=b[i-1]&0x7f;
			}
			s[j++]=0;
			printu("%s",s);

/*
 *	0x0b - StuffText
 */

		} else if(type==0x0b) {
			char *cmd;
			int j=0;

			memcpy(dbuf+dlen,b+i_start,i-i_start-1);
			dlen+=(i-i_start-1);

			while(b[i++]) {
				s[j++]=b[i-1];
			}
			s[j++]=0;
//			print("StuffText: %s",s);
			cmd=strstr(s,"precache");
			if(cmd) {
				online=true;
				qbLog("Precache completed");
				dbuf[dlen++]=0x0b;
				memcpy(dbuf+dlen,"precache\n",9);
				dlen+=9;
				dbuf[dlen++]=0x00;
			} else {
				cmd=strstr(s,"cmd");
				if(cmd) {
					int x=strlen((char *)(cmd+4));
					*((char *)(cmd+x+3))='\0';
					qbAsynchConsole((char *)(cmd+4));
//					print("%s\n",cmd+4);
				} else {
					dbuf[dlen++]=0x0b;
					memcpy(dbuf+dlen,s,j);
					dlen+=j;
				}
			}

			i_start=i;

/*
 *	0x0c - Serverinfo
 */

		} else if(type==0x0c) {
			int version;

			version=*((int *)(b+i));
			i+=4;
			qbLog("Protocol version: %d",version);
			login_key=*((int *)(b+i));
			i+=4;
			qbLog("Login key: %d",login_key);
			b[i++]=1;
			while(b[i++]);
			player_num=*((short *)(b+i))+1;
			i+=2;
			qbLog("Player Number: %d",player_num);
			while(b[i++]);

/*
 *	0x0d - ConfigStrings
 */

		} else if(type==0x0d) {
			int num;
			int j=0;

			num=*((short *)(b+i));
			i+=2;
			while(b[i]) {
				cstrings[num][j++]=b[i++];
			}
			cstrings[num][j++]=b[i++];
			cstrings[num][63]=0;
//			print("Configstring %d: %s\n",num,cstrings[num]);
			if(num==30) {
				max_players=atoi(cstrings[num]);
			}

/*
 *	0x0e - Spawn Entity
 */

		} else if(type==0x0e) {
			unsigned int mask;
			int entity;
 
			mask=b[i++];
			if(mask&0x00000080) mask|=(b[i++]<<8);
			if(mask&0x00008000) mask|=(b[i++]<<16);
			if(mask&0x00800000) mask|=(b[i++]<<24);
			if(mask&0x00000100) {
				entity=*((short *)(b+i));
				i+=2;
			} else {
				entity=b[i++];
			}
			if(entity>=1024) {
				qbLog("Entity > 1024");
				return 1;
			}
			spawn->entities[entity].modelindex=(mask&0x00000800) ? b[i++] : 0;
			spawn->entities[entity].modelindex2=(mask&0x00100000) ? b[i++] : 0;
			spawn->entities[entity].modelindex3=(mask&0x00200000) ? b[i++] : 0;
			spawn->entities[entity].modelindex4=(mask&0x00400000) ? b[i++] : 0;
			spawn->entities[entity].framenum=0;
			if(mask&0x00000010) spawn->entities[entity].framenum=b[i++];
			if(mask&0x00020000) {
				spawn->entities[entity].framenum=*((short *)(b+i));
				i+=2;
			}
			if(mask&0x00010000) {
				if(mask&0x02000000) {
					i+=4;
				} else {
					i++;
				}
			} else {
				if(mask&0x02000000) {
					i+=2;
				}
			}
			if(mask&0x00004000) {
				if(mask&0x00080000) {
					i+=4;
				} else {
					i++;
				}
			} else {
				if(mask&0x00080000) i+=2;
			}
			if(mask&0x00001000) {
				if(mask&0x00040000) {
					spawn->entities[entity].renderfx=*((int *)(b+i));
					i+=4;
				} else {
					spawn->entities[entity].renderfx=b[i++];
				}
			} else {
				if(mask&0x00040000) {
					spawn->entities[entity].renderfx=*((short *)(b+i));
					i+=2;
				} else {
					spawn->entities[entity].renderfx=0;
				}
			}
			if(mask&0x00000001) {
				spawn->entities[entity].origin[0]=0.125*((float)*((short *)(b+i)));
				i+=2;
			} else {
				spawn->entities[entity].origin[0]=0;
			}
			if(mask&0x00000002) {
				spawn->entities[entity].origin[1]=0.125*((float)*((short *)(b+i)));
				i+=2;
			} else {
				spawn->entities[entity].origin[1]=0;
			}
			if(mask&0x00000200) {
				spawn->entities[entity].origin[2]=0.125*((float)*((short *)(b+i)));
				i+=2;
			} else {
				spawn->entities[entity].origin[2]=0;
			}
			spawn->entities[entity].angles[0]=(mask&0x00000400) ? (PI/128.0*(float)b[i++]) : 0;
			spawn->entities[entity].angles[1]=(mask&0x00000004) ? (PI/128.0*(float)b[i++]) : 0;
			spawn->entities[entity].angles[2]=(mask&0x00000008) ? (PI/128.0*(float)b[i++]) : 0;
			if(mask&0x01000000) i+=6;
			if(mask&0x04000000) i++;
			if(mask&0x00000020) i++;
			if(mask&0x08000000) i+=2;
			spawn->entities[entity].updated=false;
			data_points[entity].timestamp=timestamp;
			data_points[entity].origin[0]=spawn->entities[entity].origin[0];
			data_points[entity].origin[1]=spawn->entities[entity].origin[1];
			data_points[entity].origin[2]=spawn->entities[entity].origin[2];

/*
 *	0x0f - CenterPrint
 */

		} else if(type==0x0f) {
			while(b[i++]);

/*
 *	0x10 - Download
 */

		} else if(type==0x0f) {
			int size;
			int percent;

			size=*((unsigned short *)(b+i));
			i+=2;
			percent=b[i++];
			if(size>-1) {
				i+=size;
			}

/*
 *	0x11 - Playerinfo
 */

		} else if(type==0x11) {
			unsigned int mask;
			int j;

			mask=*((unsigned short *)(b+i));
			i+=2;
			if(mask&0x0001) i++;
			if(mask&0x0002) {
				cs->player.origin[0]=0.125*((float)*((short *)(b+i)));
				i+=2;
				cs->player.origin[1]=0.125*((float)*((short *)(b+i)));
				i+=2;
				cs->player.origin[2]=0.125*((float)*((short *)(b+i)));
				i+=2;
			}
			if(mask&0x0004) {
				cs->player.velocity[0]=0.0125*((float)*((short *)(b+i)));
				i+=2;
				cs->player.velocity[1]=0.0125*((float)*((short *)(b+i)));
				i+=2;
				cs->player.velocity[2]=0.0125*((float)*((short *)(b+i)));
				i+=2;
			}
			if(mask&0x0008) i++;
			if(mask&0x0010) i++;
			if(mask&0x0020) i+=2;
			if(mask&0x0040) {
				cs->player.angles[0]=PI/32768.0*((float)*((short *)(b+i)));
				i+=2;
				cs->player.angles[1]=PI/32768.0*((float)*((short *)(b+i)));
				i+=2;
				cs->player.angles[2]=PI/32768.0*((float)*((short *)(b+i)));
				i+=2;
			}
			if(mask&0x0080) i+=3;
			if(mask&0x0100) i+=6;
			if(mask&0x0200) i+=3;
			if(mask&0x1000) cs->player.gunindex=b[i++];
			if(mask&0x2000) i+=7;
			if(mask&0x0400) i+=4;
			if(mask&0x0800) i++;
			if(mask&0x4000) i++;
			mask=*((unsigned long *)(b+i));
			i+=4;
			for(j=0;j<32;j++) {
				if(mask & (0x00000001 << j)) {
					if(j==13) {
						*((short *)(b+i))=0;
					}
					cs->player.stats[j]=*((short *)(b+i));
					i+=2;
				}
			}

/*
 *	0x12 - Entity Update
 */

		} else if(type==0x12) {
			unsigned int mask;
			int entity;
			vec3_t oldorigin;
			float f;

			while(1) {
				mask=b[i++];
				if(mask&0x00000080) mask|=(b[i++]<<8);
				if(mask&0x00008000) mask|=(b[i++]<<16);
				if(mask&0x00800000) mask|=(b[i++]<<24);
				if(mask&0x00000100) {
					entity=*((unsigned short *)(b+i));
					i+=2;
				} else {
					entity=b[i++];
				}
				if(!entity) break;
				if(entity>=1024) {
					qbLog("Entity > 1024");
					return 1;
				}
				cs->entities[entity].updated=true;
				if(mask&0x00000800) cs->entities[entity].modelindex=b[i++];
				if(mask&0x00100000) cs->entities[entity].modelindex2=b[i++];
				if(mask&0x00200000) cs->entities[entity].modelindex3=b[i++];
				if(mask&0x00400000) cs->entities[entity].modelindex4=b[i++];
				if(mask&0x00000010) cs->entities[entity].framenum=b[i++];
				if(mask&0x00020000) {
					cs->entities[entity].framenum=*((short *)(b+i));
					i+=2;
				}
				if(mask&0x00010000) {
					if(mask&0x02000000) {
						i+=4;
					} else {
						i++;
					}
				} else {
					if(mask&0x02000000) {
						i+=2;
					}	
				}
				if(mask&0x00004000) {
					if(mask&0x00080000) {
						i+=4;
					} else {
						i++;
					}
				} else {
					if(mask&0x00080000) i+=2;
				}
				if(mask&0x00001000) {
					if(mask&0x00040000) {
						cs->entities[entity].renderfx=*((int *)(b+i));
						i+=4;
					} else {
						cs->entities[entity].renderfx=b[i++];
					}
				} else {
					if(mask&0x00040000) {
						cs->entities[entity].renderfx=*((short *)(b+i));
						i+=2;
					}
				}
				if(mask&0x00000001) {
					cs->entities[entity].origin[0]=0.125*((float)*((short *)(b+i)));
					i+=2;
				}
				if(mask&0x00000002) {
					cs->entities[entity].origin[1]=0.125*((float)*((short *)(b+i)));
					i+=2;
				}
				if(mask&0x00000200) {
					cs->entities[entity].origin[2]=0.125*((float)*((short *)(b+i)));
					i+=2;
				}
				f=0.01*(float)(timestamp-data_points[entity].timestamp);
				if(f>0.0 && f<=10.0) {
					cs->entities[entity].velocity[0]=(cs->entities[entity].origin[0]-data_points[entity].origin[0])/f;
					cs->entities[entity].velocity[1]=(cs->entities[entity].origin[1]-data_points[entity].origin[1])/f;
					cs->entities[entity].velocity[2]=(cs->entities[entity].origin[2]-data_points[entity].origin[2])/f;
				} else {
					cs->entities[entity].velocity[0]=0;
					cs->entities[entity].velocity[1]=0;
					cs->entities[entity].velocity[2]=0;
				}
				data_points[entity].timestamp=timestamp;
				data_points[entity].origin[0]=cs->entities[entity].origin[0];
				data_points[entity].origin[1]=cs->entities[entity].origin[1];
				data_points[entity].origin[2]=cs->entities[entity].origin[2];
				if(mask&0x00000004) cs->entities[entity].angles[0]=(PI/128.0*(float)b[i++]);
				if(mask&0x00000400) cs->entities[entity].angles[1]=(PI/128.0*(float)b[i++]);
				if(mask&0x00000008) cs->entities[entity].angles[2]=(PI/128.0*(float)b[i++]);
				if(mask&0x01000000) i+=6;
				if(mask&0x04000000) i++;
				if(mask&0x00000020) i++;
				if(mask&0x08000000) i+=2;
				if(mask&0x00000040) {
					cs->entities[entity].updated=false;
				}
			}

/*
 *	0x14 - Frame Update
 */

		} else if(type==0x14) {
			int count;

			last_frame=current_frame;
			current_frame=*((unsigned long *)(b+i));
			i+=4;
			delta_frame=*((unsigned long *)(b+i));
			i+=4;
			i+=1;
			count=b[i++];
			i+=count;
			if((current_frame-last_frame)>12) {
				current_state=0;
			} else {
				current_state=(current_state+current_frame-last_frame)%16;
			}
			if(delta_frame==0xffffffff) {
				ds=&(states[16]);
				packet_loss=0;
			} else if((current_frame-delta_frame)>12) {
				qbLog("*** Too much packet loss ***");
				packet_loss=0x80000000;
				return 0;
			} else {
				ds=&(states[(current_state+delta_frame-current_frame+16)%16]);
				packet_loss=0;
			}
			cs=&(states[current_state]);
			memcpy(cs,ds,sizeof(igamestate_t));

/*
 *	0x?? - Undefined
 */

		} else {
			qbLog("*** BAD server packet type: %02Xh ***",type);
			hexdump(b,len);
			return 0;
		}
	}
	if(i>len) {
		qbLog("*** Server Packet Misalignment Error ***");
		hexdump(b,len);
		return 0;
	}

	memcpy(dbuf+dlen,b+i_start,i-i_start);
	dlen+=(i-i_start);
	i_start=i;

	if(recording) {
		write(demo_handle,&dlen,4);
		write(demo_handle,dbuf,dlen);
	}

	return 0;
}

void qbMovementOrder(vec3_t angles, vec3_t velocity, bool attack) {
	vec3_t adj_angles;
	int frameTime;

	if(!online) {
		return;
	}

	LOCK(lock);

	frameTime=_GetTickCount()-lastUpdateTime;
	while(frameTime<10) {
		Sleep(10);
		frameTime=_GetTickCount()-lastUpdateTime;
	}
	lastUpdateTime+=frameTime;
	if(frameTime>255) frameTime=255;

	adj_angles[0]=angles[0]+cs->player.angles[0];
	adj_angles[1]=angles[1]-cs->player.angles[1];
	adj_angles[2]=angles[2];

	updates[current_update].angles[0]=adj_angles[0];
	updates[current_update].angles[1]=adj_angles[1];
	updates[current_update].angles[2]=adj_angles[2];
	updates[current_update].velocity[0]=velocity[0];
	updates[current_update].velocity[1]=velocity[1];
	updates[current_update].velocity[2]=velocity[2];
	updates[current_update].msec=frameTime;
	updates[current_update].light=0;
	updates[current_update].buttons=(attack) ? 0x81 : 0;
	updates[current_update].impulse=0;
	updates[current_update].timestamp=_GetTickCount();

	netMovementOrder();
	
	UNLOCK(lock);
}

void qbGetGameState(gamestate_t *s) {
	int i;
	float f;
	entity_t *d,*e;
	
	if(!online) {
		return;
	}
	
	LOCK(lock);
	
	f=0.01*(float)(runningPing+_GetTickCount()-cs->timestamp);

	s->player.sure_origin[0]=cs->player.origin[0];
	s->player.sure_origin[1]=cs->player.origin[1];
	s->player.sure_origin[2]=cs->player.origin[2];
	s->player.origin[0]=cs->player.origin[0]+f*cs->player.velocity[0];
	s->player.origin[1]=cs->player.origin[1]+f*cs->player.velocity[1];
	s->player.origin[2]=cs->player.origin[2]+f*cs->player.velocity[2];
	s->player.velocity[0]=cs->player.velocity[0];
	s->player.velocity[1]=cs->player.velocity[1];
	s->player.velocity[2]=cs->player.velocity[2];
	s->player.angles[0]=cs->player.angles[0];
	s->player.angles[1]=cs->player.angles[1];
	s->player.angles[2]=cs->player.angles[2];
	
	s->player.health=cs->player.stats[1];
	strcpy(s->player.ammo_icon,cstrings[cs->player.stats[2]+544]);
	s->player.ammo=cs->player.stats[3];
	strcpy(s->player.armor_icon,cstrings[cs->player.stats[4]+544]);
	s->player.armor=cs->player.stats[5];
	strcpy(s->player.weapon_model,cstrings[cs->player.gunindex+32]);
	s->player.timer=cs->player.stats[10];
	s->player.frags=cs->player.stats[14];

	for(i=0;i<1024;i++) {
		e=&(cs->entities[i]);
		d=&(s->entities[i]);
		d->origin[0]=e->origin[0]+f*e->velocity[0];
		d->origin[1]=e->origin[1]+f*e->velocity[1];
		d->origin[2]=e->origin[2]+f*e->velocity[2];
		d->velocity[0]=e->velocity[0];
		d->velocity[1]=e->velocity[1];
		d->velocity[2]=e->velocity[2];
		d->angles[0]=e->angles[0];
		d->angles[1]=e->angles[1];
		d->angles[2]=e->angles[2];
		d->modelindex=e->modelindex;
		d->framenum=e->framenum;
		d->updated=e->updated;
	}

	s->player_entity=player_num;
	s->max_players=max_players;
	
	UNLOCK(lock);
}

int qbGetInventory(inventory_t *x) {
	int i;

	if(!online) {
		return 0;
	}

	LOCK(lock);

	for(i=0;i<256;i++) {
		strcpy((*x)[i].name,cstrings[1056+i]);
		(*x)[i].number=inventory[i];
	}
	
	UNLOCK(lock);
	
	return _GetTickCount()-lastInvenTime;
}

bool qbGetModelString(int i, char *s) {

	if(!online) {
		return false;
	}
	
	LOCK(lock);
	
	if(i>=0 && i<256) {
		strcpy(s,cstrings[i+32]);
		UNLOCK(lock);
		return (s[0]!='\0');
	}
	
	UNLOCK(lock);
	
	return false;
}

bool qbGetPlayerName(int i, char *s) {

	if(!online) {
		return false;
	}

	LOCK(lock);
	if(i>0 && i<=256) {
		strcpy(s,cstrings[i+1311]);
		UNLOCK(lock);
		return (s[0]!='\0');
	}
	UNLOCK(lock);
	
	return false;
}	

#if WIN32
DWORD serverThread(void* ptr) {
#else
void* serverThread(void *ptr) {
#endif
	char buffer[2048];
	int length;
	int result;

	do {
		int ms = qbTickCount();
		
		LOCK(lock);
		length=recvPacket(buffer);
		if(length>0) {
			result=processServerPacket((unsigned char *)buffer,length);
			cs->timestamp=_GetTickCount();
		} else {
			result=0;
		}
		UNLOCK(lock);

		int elapsed = qbTickCount() - ms;
		throttle( elapsed, QB_MIN_FRAMETIME );
		
	} while(result==0 && ! kill_server_thread);
	if(online) {
		qbLog("*** Server Disconnected ***");
	}
	online=false;
	return 0;
}

int qbPingTime(void) {
	return runningPing;
}

void qbRecordDemo(const char *filename) {
	recording=false;
	if(!online) {
#ifdef WIN32  
		HANDLE hFile=CreateFile(filename,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile==INVALID_HANDLE_VALUE) {
			printe("Error: Could not create %s\n",filename);
			return;
		}
		CloseHandle(hFile);
#endif
		demo_handle=open(filename,O_BINARY|O_WRONLY|O_TRUNC|O_CREAT);
		if(demo_handle==-1) {
			qbLog("Error: Could not open %s",filename);
			return;
		}
		qbLog("Recording demo: %s",filename);
		recording=true;
	} else {
		qbLog("Can't record demo while connected");
	}
}

void qbSetPrintFunc(PRINTFUNC p) {
	setPrintFunc3(p);
}

bool qbStartup(PRINTFUNC p1,PRINTFUNC p2) {

	setPrintFuncs(p1,p2);

	qbLog("--- Starting Server ---");
	qbLog(VERSION_STRING);

	memset(cstrings,0,1568*sizeof(configstring_t));
	memset(data_points,0,1024*sizeof(time_point_t));
	memset(&(states[16]),0,sizeof(igamestate_t));

	netPreInit();

	return true;
}

bool qbConnect(const char *hostname, unsigned short port) {
	if(!netInit(hostname,port)) {
		return false;
	}

#if WIN32  
  server_thread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)serverThread,NULL,0,&server_thread_id);
  if(server_thread == NULL) {
#else	
	int ret = pthread_create(&server_thread, NULL, serverThread, NULL);
	if( ret != 0 ) {
#endif  
		qbLog("Could not create server thread");
		netShutdown();
		return false;
	}
/*TODO use pthread_attribs to set higher priority*/
#ifdef WIN32
	SetThreadPriority(server_thread,THREAD_PRIORITY_TIME_CRITICAL);
#endif

	if(!netConnect()) {
		qbLog("Could not connect to server");
		netShutdown();
		return false;
	}

	qbSynchConsole("new");
	while(!online) {
		qbLog("Precaching...");
		Sleep(1000);
	}
	return true;
}

void qbBegin(void) {
	char s[64];
	sprintf(s,"begin %d\n",login_key);
	qbSynchConsole(s);
}

bool qbIsConnected(void) {
	return online;
}

void qbDisconnect(void) {
	if(online) {
		online=false;
		qbSynchConsole("disconnect");
    kill_server_thread = true;
/*		TerminateThread(server_thread,0);*/
	}
	qbLog("*** Client Disconnected ***\n");
}

void qbShutdown(void) {
	if(recording) {
		int length=-1;
		write(demo_handle,&length,4);
		close(demo_handle);
		recording=false;
	}
	netShutdown();
}

void qbGetBrushLocation(int n, vec3_t *p) {
	char modelname[8];
	float f;
	int modelindex;
	int i;

	(*p)[0]=0.0;
	(*p)[1]=0.0;
	(*p)[2]=0.0;

	if(!online) {
		return;
	}
	
//	while(lock) {
//		Sleep(10);
//	}
//	LOCK(lock);
	if(n>256) return;

	modelname[0]='*';
  snprintf(&(modelname[1]), 6, "%d", n);
//	itoa(n,&(modelname[1]),10);

	for(modelindex=0;modelindex<256;modelindex++) {
		if(strcmp(cstrings[modelindex+32],modelname)==0) break;
	}
	if(modelindex==256) return;

	f=0.01*(float)(runningPing+_GetTickCount()-cs->timestamp);

	for(i=max_players+1;i<1024;i++) {
		if(cs->entities[i].modelindex==modelindex) {
			(*p)[0]=cs->entities[i].origin[0]+f*cs->entities[i].velocity[0];
			(*p)[1]=cs->entities[i].origin[1]+f*cs->entities[i].velocity[1];
			(*p)[2]=cs->entities[i].origin[2]+f*cs->entities[i].velocity[2];
			return;
		}
	}
//	UNLOCK(lock);
}

void qbSetTraceLineFunc(TRACELINEFUNC t) {
	;
}

int qbGetAPIVersion(void) {
	return Q2BOT_API_VERSION;
}
