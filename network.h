/*
 * network.h
 *
 * Copyright (C) 1999, Ben Swartzlander
 * This file is part of Q2BotCore.
 * For conditions of distribution and use,
 * see the accompanying README file.
 */

#ifndef __NETWORK_H
#define __NETWORK_H

#define UPDATE_LIST_LENGTH 128

typedef struct {
	vec3_t angles;
	vec3_t velocity;
	unsigned char msec,light,buttons,impulse;
	int timestamp;
} ucmd_t;

void netPreInit(void);
bool netInit(const char *,unsigned short);
bool netConnect(void);
void netShutdown(void);

void netMovementOrder();
int recvPacket(char *);

#endif

