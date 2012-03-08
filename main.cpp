#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#ifdef WIN32
#	include <windows.h>

char* strsep(char** s, const char* delim) {

	char* p = *s;

	if( !p )
		return NULL;
	
	int len = strlen(*s);
	if( len <= 0 ) {
		*s = NULL;
		return NULL;
	}
	
	while( 1 ) {
		int i;
		for( i=0; i<strlen(delim); i++ ) {
			if( *p == delim[i] || !*p ) {
				char* stringp = *s;

				// Null out the delimiting char
				*p = '\0';

				// Update the passed in pointer to start at the char after the match
				if( p < *s + len )			
					*s = p + 1;
				else
					*s = NULL;
				
				return stringp;
			}
		}
		p++;
	}
	return NULL;
}

#else
#	include <unistd.h>
#	include <sys/select.h>
#	include <poll.h>
#endif
#include "q2bot.h"
#include "gpbot.h"

#include "gamestate.h"
#include "ammo.h"
#include "armor.h"
#include "entities.h"
#include "health.h"
#include "player.h"
#include "powerup.h"
#include "projectile.h"
#include "weapon.h"

// Per-frame functions; one of these is called once per frame depending on state 
struct BotThinkFunc* botIdle(struct BotState* state);
struct BotThinkFunc* botRun(struct BotState* state);
struct BotThinkFunc* botShutdown(struct BotState* state);

struct BotThinkFunc botThinkFuncs[] = {
	{ IDLE, botIdle },
	{ RUN, botRun },
	{ SHUTDOWN, botShutdown }
};

// Command handler functions
static struct BotThinkFunc* botConnect(int argc, const char* argv[], struct BotState* state);
static struct BotThinkFunc* botDisconnect(int argc, const char* argv[], struct BotState* state);
static struct BotThinkFunc* botStart(int argc, const char* argv[], struct BotState* state);
static struct BotThinkFunc* botStop(int argc, const char* argv[], struct BotState* state);
static struct BotThinkFunc* botQuit(int argc, const char* argv[], struct BotState* state);
 
static struct BotCmd botCmds[] = {
	{ "connect", 2, botConnect },
	{ "disconnect", 0, botDisconnect },
	{ "start", 0, botStart },
	{ "stop", 0, botStop },
	{ "quit", 0, botQuit }
};

//
// Send a movement command to the quake2 server. In the future this will also
// record some statistics by inspecting the move command in relation to the
// game state
void botMovementOrder(gamestate_t* gs, vec3_t angles, vec3_t velocity, int fire) {

	qbLog("angles=(%3.1f %3.1f %3.1f), velocity=(%3.1f %3.1f %3.1f), fire=%d", angles[0], angles[1], angles[2], velocity[0], velocity[1], velocity[2], fire);
	qbMovementOrder(angles, velocity, fire);
	
	// Ideas:
	// - are we aiming at anything meaningful?
	// --> are we firing at it?
	// - are we close to anything/anyone meaningful?
	// - what weapons do we have?
	// --> ammo?
	// --> health?
	// - can we somehow tell if we are constantly walking into walls?
	// --> specifically, are we moving anywhere at all vs. plowing into nothing?
}

struct BotThinkFunc* botDispatchCmd(char* cmdLine, struct BotState* state) {

	const int MAX_ARGS = 256;
	const char* argv[MAX_ARGS];
	int argc = 0;
	char* s = cmdLine; 

	// Trim the input
	while( isspace(*s) )
		s++;
	while( isspace(s[strlen(s)-1]) )
		s[strlen(s)-1] = '\0';
		
	qbLog("Dispatching command: %s", cmdLine);

	// Compute argc,argv
	while( s ) {
		argv[argc] = strsep(&s, " \n");
		argc++;
	}
	
	// Search for the command
	for( int i=0; i<sizeof(botCmds)/sizeof(botCmds[0]); i++ ) {
		
		struct BotCmd* cmd = &botCmds[i];
		
		if( !strcmp(cmd->name, argv[0]) ) {
			
			if( argc == 1+cmd->argc )
				return cmd->call(argc, argv, state);
		}
	}
	
	// Didn't find the command
	qbLog("Received unknown command: %s", cmdLine);
	return NULL;
}

int main(int argc, const char* argv[]) {
	
	struct BotState state; memset(&state, 0, sizeof(state));
	state.name = argv[1];

	qbSetLogFile("log");
	qbStartup(NULL, NULL);

	// Initialize state
	state.func = &botThinkFuncs[IDLE];
	state.self = &state.gs.player;
	
	// Register entity types
	registerEntityTypes();
	
	while(1) {
		
		char cmd[128];	memset(cmd, 0, sizeof(cmd));
		
		int frameStart = qbTickCount();
		
		// Run the bot current stateFunc
		if( state.func ) {
			struct BotThinkFunc* nextFunc = state.func->call(&state);
			if( nextFunc ) 
				state.func = nextFunc;
		}
				
		int frameElapsed = qbTickCount() - frameStart;
		// Compute the max timeout (in milliseconds) based on the time the frame took	
		int timeout = QB_MIN_FRAMETIME - frameElapsed;
		if( timeout < 0 )
			timeout = 0;
		
		// Check if we have any input waiting for us on stdin
		qbLog("Polling stdin for command(s). Waiting %d ms", timeout);
#ifdef WIN32
		HANDLE STDIN = GetStdHandle(STD_INPUT_HANDLE);
		// Poll stdin for input
		if( WaitForSingleObject(STDIN, timeout) == WAIT_OBJECT_0 )
			fgets(cmd, sizeof(cmd)-1, stdin);
#else
		// Setup the polling struct for stdin
		struct pollfd in;
		
		in.fd = STDIN_FILENO;
		in.events = POLLIN;
			
		// Poll stdin
		if( poll(&in, 1, timeout) > 0 
		&& (in.revents & POLLIN) != 0 )
			fgets(cmd, sizeof(cmd)-1, stdin);
#endif

		// If we don't have any command to process, just move on to the
		// next iteration
		if( !strlen(cmd) )
			continue;
	
		struct BotThinkFunc* nextFunc = botDispatchCmd(cmd, &state);
		if( nextFunc ) 
			state.func = nextFunc;
	}
}

// Think functions ////////////////////////////////////////////////////////////

struct BotThinkFunc* botIdle(struct BotState* state) {

	static int nextPulse = 0;
	static const int pulseTimeout = 1000;

	vec3_t angles;
	vec3_t velocity;
	 
	angles[0] = 0.0; angles[1] = 0.0; angles[2] = 0.0;
	velocity[0] = 0.0; velocity[1] = 0.0; velocity[2] = 0.0;
	qbMovementOrder(angles, velocity, 0);

	if( nextPulse < qbTickCount() ) {
		qbLog("heartbeat");
		nextPulse = qbTickCount() + pulseTimeout;
	}

#ifdef WIN32
	Sleep(QB_MIN_FRAMETIME);
#else
	usleep(QB_MIN_FRAMETIME * 1000);
#endif
	return NULL;
}

// botRun is provided by the individual

struct BotThinkFunc* botShutdown(struct BotState* state) {
	qbLog("Shutting down bot core...");
	qbShutdown();
	qbLog("Ok, exiting.");
	exit(EXIT_SUCCESS);	
}

// Command handlers //////////////////////////////////////////////////////////

struct BotThinkFunc* botConnect(int argc, const char* argv[], struct BotState* state) {
	
	state->host = argv[1];
	state->port = atoi(argv[2]);
	state->demoName = argc >= 4 ? argv[3] : NULL;
		
	if( state->demoName ) {
		qbLog("%s: Recording demo: %s", state->name, state->demoName);
		qbRecordDemo(state->demoName);
	}
	
	qbLog("Setting username: %s", state->name);	
	qbUpdateUserinfo("name", state->name);

	qbLog("Attempting connection to server: %s:%d", state->host, state->port);
	if( !qbConnect(state->host, state->port) )
		RETURN(IDLE,"Could not connect to server");

	qbLog("Connected to server, entering game.");
	qbBegin();
	
	qbLog("Ready, waiting for combat"); 
	RETURN(IDLE,"ok");
}

struct BotThinkFunc* botDisconnect(int argc, const char* argv[], struct BotState* state) {

	qbLog("Disconnecting bot from server...");	
	qbDisconnect();
	qbLog("Disconnected.");	
	RETURN(IDLE,"ok");
}

struct BotThinkFunc* botStart(int argc, const char* argv[], struct BotState* state) {
	qbLog("Commencing combat");
	RETURN(RUN,"ok");
}

struct BotThinkFunc* botStop(int argc, const char* argv[], struct BotState* state) {
	qbLog("Stopping combat");
	RETURN(IDLE,"ok");
}

struct BotThinkFunc* botQuit(int argc, const char* argv[], struct BotState* state) {

	char stats[4096];
	player_t* self = state->self;

	qbLog("Dumping results to stdout...");	
	// Dump stats to stdout
	sprintf(stats, "{ 'frags':%d, 'health':%d, 'armor':%d, 'ammo':%d }", 
			self->frags, self->health, self->armor, self->ammo);

	qbLog("Shutting down.");	
	RETURN(SHUTDOWN,stats);
}

 