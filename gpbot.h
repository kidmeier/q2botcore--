#ifndef GPBOT_H_
#define GPBOT_H_

#define DIV(a,b) (((b) == 0.0) ? 1.0 : ((a) / (b)))

enum BotState_e {
	IDLE,
	RUN,
	SHUTDOWN
};
struct BotThinkFunc {
	BotState_e id;
	struct BotThinkFunc* (*call)(struct BotState*);
};

struct BotState {
	const char* 	name;
	const char*		host;
	int				port;
	const char*		demoName;
	
	gamestate_t		gs;
	player_t*		self;
	
	BotThinkFunc*		func;
};

struct BotCmd {
	const char* name;
	int argc;
	struct BotThinkFunc* (*call)(int, const char**, struct BotState*);
};
#define RETURN(state,result)	{ printf("return %s\n", (result)); fflush(stdout); qbLog("Entering state: %s", #state); return &botThinkFuncs[(state)]; }

extern struct BotThinkFunc botThinkFuncs[];
extern void botMovementOrder(gamestate_t* gs, vec3_t angles, vec3_t velocity, int fire);

#endif /*GPBOT_H_*/
