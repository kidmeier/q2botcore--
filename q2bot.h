/*
 * q2bot.h
 *
 * Copyright (C) 1999, Ben Swartzlander
 * This file is part of Q2BotCore.
 * For conditions of distribution and use,
 * see the accompanying README file.

 * This is the header file for the functions in
 * q2bot.dll
 */

#ifndef __Q2BOT_H
#define __Q2BOT_H

#define Q2BOT_API_VERSION 14

#if defined(__cplusplus)
extern "C"
{
#endif

typedef float vec3_t[3];

/*
 *	player_t
 *
 *	Contains info about the player (you)
 *
 *	angles[0]=pitch [-PI/2 to PI/2]
 *	angles[1]=yaw [0 to 2*PI]
 *	angles[2]=roll (not important)
 *	velocity[0]=forward [-400 to 400]
 *	velocity[1]=right [-400 to 400]
 *	velocity[2]=up [-400 to 400]
 *
 *	The difference between origin and
 *	sure_origin is that sure_origin is the last
 *	origin sent by the server (garaunteed to be
 *	in-bounds) while origin is affected by
 *	build in prediction, and may be inside a
 *	wall.
 *
 *	The timer is for things like quad and
 *	invulnerability.
 */

typedef struct {
	vec3_t angles;
	vec3_t origin;
	vec3_t velocity;
	vec3_t sure_origin;
	int health;
	char ammo_icon[64];
	int ammo;
	char armor_icon[64];
	int armor;
	char weapon_model[64];
	int timer;
	int frags;
} player_t;

/*
 *	entity_t
 *
 *	Contains info about anything that moves/exists
 *
 *	The meaning of modelindexes can be looked up with
 *	qbGetModelString(). You're on your own with
 *	framenum and renderfx. The updated flag tells you
 *	whether or not the entity is visible. If it is
 *	true, you can be sure it's there. If it is false,
 *	it may be out of sight, or unspawned.
 */

typedef struct {
	vec3_t angles;
	vec3_t origin;
	vec3_t velocity;
	int modelindex;
	int modelindex2;
	int modelindex3;
	int modelindex4;
	int framenum;
	int renderfx;
	bool updated;
} entity_t;

/*
 *	gamestate_t
 *
 *	Contains a player and up to 1024 entities
 *
 *	The entity number of the player is included
 *	so you can ignore that entity for AI purposes.
 *	The max_players field is an indication of
 *	which entities are player and which are other
 *	stuff.
 *
 *	entities[1] through entites[max_players] are
 *	player entites.
 *
 *	entities[max_players+1] through entites[1023]
 *	are other entities.
 *
 *	entities[0] appears to be unused.
 */

typedef struct {
	player_t player;
	int player_entity;
	int max_players;
	entity_t entities[1024];
} gamestate_t;

/*
 *	item_t
 *
 *	Contains info about an item in your inventory,
 *	including its name and how many you've got
 */

typedef struct {
	char name[64];
	int number;
} item_t;

/*
 *	inventory_t
 *
 *	An array of up to 256 items in you inventory
 */

typedef item_t inventory_t[256];

/*
 *	PRINTFUNC
 *
 *	A prototype for a function of the form:
 *	void x(char *);
 *	You must pass two such functions to the
 *	startup function;
 */

typedef void (*PRINTFUNC)(char *);

/*
 *	TRACELINEFUNC
 *
 *	A prototype for a function of the form:
 *	float x(vec3_t,vec3_t,int);
 *	A function of this sort can be provided to
 *	enhance certain algorithms with knowledge of
 *	the map.
 */

typedef float (*TRACELINEFUNC)(vec3_t,vec3_t,int);

/*
 *	qbGetAPIVersion()
 *
 *	This function should have been added earlier. It
 *	just occured to me that it is possible to get one's
 *	DLL's and EXE's mixed up, and if they're not
 *	compatible, gibs will fly. This allows the EXE to
 *	check the version of the DLL before calling any
 *	functions.
 *
 *	Returns: Q2BOT_API_VERSION (#defined above)
 */

int qbGetAPIVersion(void);

/*
 *	qbStartup(p1,p2)
 *
 *	p1: function to print normal messages
 *	p2: function to print errors
 *
 *	This function initialized internal data and
 *	sets up the print functions. These flexible
 *	print functions are useful if you want to
 *	supress output, or redirect it somewhere
 *	else. There are two functions, one for
 *	normal messages, one for errors, right now
 *	most errors go to normal output, but this
 *	will be changeg.
 *	If this function fails, you must abort, or
 *	else the results are unpredictable.
 *
 *	Return: true on success
 *	        false on failure
 */

bool qbStartup(PRINTFUNC,PRINTFUNC);

/*
 *	qbConnect(hostname,port)
 *
 *	hostname: string representing an IP address
 *	          or a hostname, where the server is
 *	port: the port to connect to (usually 27910)
 *
 *	This function connects to the server and
 *	causes data to begin flowing.
 *	If this function fails, you must abort, or
 *	else the results are unpredictable.
 *
 *	Return: true on success
 *	        false on failure
 */

bool qbConnect(const char *,unsigned short);

/*
 *	qbIsConnected()
 *
 *	This fuction tell whether we are still
 *	connected to the server.
 *
 *	Return: true if connected
 *	        false if disconnected
 */

bool qbIsConnected(void);

/*
 *	qbDisconnect()
 *
 *	This function disconnects from the server.
 *	Calls this before calling shutdown(),
 *	unless you never connected.
 */

void qbDisconnect(void);

/*
 *	qbShutdown()
 *
 *	This function deallocates all internal
 *	resources. You should call this last, before
 *	exiting.
 */

void qbShutdown(void);

/*
 *	qbSynchConsole(command)
 *
 *	command: a string
 *
 *	This is the normal way to send console
 *	commands. Delivery of the message is
 *	garaunteed, and the function blocks until
 *	delivery has been confimed.
 *
 *	NOTE: the name of this function has been
 *	changed from qbConsoleCommand. There is a
 *	macro below for backwards compatibility.
 */

void qbSynchConsole(const char *);
#define qbConsoleCommand qbSynchConsole

/*
 *	qbAsynchConsole(command)
 *
 *	command: a string
 *
 *	This function sends a console command
 *	much like qbSynchConsole, but it returns
 *	immediately. The message is garaunteed to
 *	be delivered in a finite amount of time,
 *	but the length of time is unknown.
 *
 *	NOTE: this function has taken the place
 *	of qbQuickConsole in the API. Because it
 *	is a functional superset of qbQuickConsole,
 *	a macro is defined below for backwards
 *	compatibility.
 */

void qbAsynchConsole(const char *);
#define qbQuickConsole qbAsynchConsole

/*
 *	qbUpdateUserinfo(key,value)
 *
 *	key: a string
 *	value: a string
 *
 *	This is how you set various userinfo key/value
 *	pairs. The Quake 2 client converts some console
 *	commands such as 'name' and 'skin' to userinfo
 *	commands. Here you must do it manually.
 *	if you call consoleCommand("name Bot") the bot
 *	will actually say this, because there is no
 *	server command 'name'. You must call
 *	updateUserinfo("name","Bot")
 *
 *	As with consoleCommand() delivery is garaunteed,
 *	but this will not return until delivery is
 *	confirmed.
 */

void qbUpdateUserinfo(const char *, const char *);

/*
 *	qbMovementOrder(angles,velocity,fire)
 *
 *	angles: a vector containing pitch, yaw, and roll
 *	        the server ignores roll
 *	velocity: your desired velocity
 *	fire: flag for the fire button
 *
 *	This is the function you use to actually move
 *	around. You may call it as often as you like;
 *	each time you do, a packet is send to the
 *	server. The more often you do this, the
 *	smoother your movement will be.
 *	Delivery is not garuanteed, but Quake 2 error
 *	correction is implemented, so up to 66% packet
 *	loss is tolerable.
 *	The components of the vectors are read like
 *	this:
 *	angles[0]=pitch [-PI/2 to PI/2]
 *	angles[1]=yaw [0 to 2*PI]
 *	angles[2]=roll (ignored)
 *	velocity[0]=forward [-400 to 400]
 *	velocity[1]=right [-400 to 400]
 *	velocity[2]=up [-400 to 400]
 *
 *	Two notes: velocity is relative to the angle
 *	you are facing, not absolute. Also, to jump
 *	and crouch you use velocity[2].
 */

void qbMovementOrder(vec3_t,vec3_t,bool);

/*
 *	qbGetGameState(gamestate)
 *
 *	gamestate: a pointer to an empty gamestate_t
 *
 *	This copies the current gamestate into the
 *	struct you pass in. Note that although the
 *	server only updates the gamestate 10 times
 *	per second, this function uses linear
 *	extrapolation to approximate the gamestate
 *	in real time. This results in smoother
 *	motion, allowing bots to realisticly operate
 *	at up to 80Hz. I recommend between 30Hz and
 *	40Hz.
 */

void qbGetGameState(gamestate_t *);

/*
 *	qbGetInventory(inventory)
 *
 *	inventory: a pointer to an empty inventory_t
 *
 *	This gets a copy of most recent verions of
 *	the inventory known. It is important to know
 *	that the players inventory is not updated
 *	automatically. It is only updated when you
 *	send the server a "inven" console command.
 *	I do not recommend doing this more than
 *	once per second, as it generates a massive
 *	512 bytes packet from the server.
 *
 *	Returns: the numer of milliseconds since
 *	         the inventory was refreshed
 */

int qbGetInventory(inventory_t *);

/*
 *	getModelString(modelindex,modelstring)
 *
 *	modelindex: the index of the model (obtained
 *	            from an entity_t
 *	modelstring: an empty string of 64 bytes
 *
 *	This is how you can find the names of the
 *	models. These are important for any real
 *	AI. Once they are set, I don't think they
 *	change for the rest of the map.
 *
 *	returns: true, if the model's string is known
 *	         false, if the server has not yet
 *	                told us this string
 */

bool qbGetModelString(int,char *);

/*
 *	qbGetPlayerName(player_num,namestring)
 *
 *	player_num: the entity number of the player
 *	namestring: an empty string of 64 bytes
 *
 *	This is how you can find the names of the
 *	players. It is also useful for determining
 *	if a player is logged in or not. Note that
 *	the model/skin is tagged on the back of the
 *	name after a backslash. The format is like
 *	this: "PlayerName\model/skin"
 *
 *	returns: true, if the player is logged in
 *	         false, if there is no player
 *	                associated with the player_num
 */

bool qbGetPlayerName(int,char *);

/*
 *	qbRecordDemo(filename)
 *
 *	filename: the filename that you want to the
 *	          demo to be stored in.
 *
 *	This function opens a file and sets the
 *	server to record into that demofile, so you
 *	can play it back in Quake2 later.
 *	It is VERY IMPORTANT that you call this
 *	before qbConnect(), otherwise it will fail.
 */

void qbRecordDemo(const char *);

/*
 *	qbBegin()
 *
 *	Call this when you are done initializing
 *	and you would like to start receiving game
 *	packets.
 */

void qbBegin(void);

/*
 *	qbPingTime()
 *
 *	This function returns the current ping time
 *	from the client to the server, in
 *	milliseconds. The ping time is calculated by
 *	measuring the length of time it takes for
 *	the server to respond to a "status" request.
 *	The ping time is updated once every roughly
 *	3 seconds, and in order to prevent erratic
 *	readings, it is averaged with the last few
 *	ping times.
 *
 *	returns: an int, representing ping time in
 *	         milliseconds
 */

int qbPingTime(void);

/*
 *	qbSetPrintFunc(p)
 *
 *	p: function to print messages from the
 *	   Quake2 server.
 *
 *	This function is used if you want to
 *	intercept messages such as players talking
 *	and other messages directly from the server.
 *	Unlike the other print functions in
 *	qbStartup(), this function will accept NULL
 *	as a parameter. That will cause Q2BotCore to
 *	discard all text messages from the Q2 server
 *	silently, and NULL is indeend the initial
 *	value for this function.
 */

void qbSetPrintFunc(PRINTFUNC);

/*
 *	qbGetBrushLocation(index,origin)
 *
 *	index: the index number of the movable brush
 *	origin: a vec3_t where the origin will be
 *	        stored
 *
 *	(ALPHA) This function is not fully tested. Use
 *	at your own risk.
 *
 *	This function was added for use with the
 *	q2map module. It is not useful for anything
 *	other than updating the map structures.
 */

void qbGetBrushLocation(int,vec3_t *);

/*
 *	qbSetTraceLineFunc(t)
 *
 *	t: a function the performs a traceLine from
 *	   one vector to another and returns the
 *	   distance until a collision.
 *
 *	(ALPHA) This function is not fully tested. Use
 *	at your own risk.
 *
 *	This function is used to allow Q2BotCore to
 *	trace lines through the map. It can enhance
 *	certain physics simulations. It is designed
 *	for use with the qmTraceLine from q2map. It
 *	is quite unnecessary however, and its use is
 *	not recommended.
 */

void qbSetTraceLineFunc(TRACELINEFUNC);

/*
 * returns the number of miliseconds elapsed according to q2botcore
 */
int qbTickCount(void); 

/*
 * logs the printf-formated string to the log file
 */
void qbSetLogFile(const char* fileName);
#define qbLog(...)	_qbLog(__FILE__, __LINE__, __func__, __VA_ARGS__) 
void _qbLog(const char* file, const int lineno, const char* func, const char *s, ...);

// Minimum time to spend in each iteration of the network loops (milliseconds)
#ifndef QB_MIN_FRAMETIME
#define QB_MIN_FRAMETIME	33
#endif

#if defined(__cplusplus)
}
#endif 

#endif

