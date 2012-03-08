/*
 * util.h
 *
 * Copyright (C) 1999, Ben Swartzlander
 * This file is part of Q2BotCore.
 * For conditions of distribution and use,
 * see the accompanying README file.
 */

#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "q2bot.h"

#define PI 3.1415926535

void setPrintFuncs(PRINTFUNC,PRINTFUNC);
void setPrintFunc3(PRINTFUNC);
void print(char *,...);
void printe(char *,...);
void printu(char *,...);

unsigned checksum(unsigned char *,int,int);
void hexdump(const unsigned char *,int);

void throttle(int elapsed, int frametime);

#if WIN32

# define LOCK(x)          \
    while( (x) )          \
      Sleep(10);          \
    (x) = true            
# define UNLOCK(x)        (x) = false

#else	// linux, Mac

void Sleep(int millis);

#	ifdef _DEBUG
# 	define LOCK(x)           printf("%s: Acquiring %s...", __func__, #x); pthread_mutex_lock(&(x)); printf("ok\n")
# 	define UNLOCK(x)         pthread_mutex_unlock(&(x)); printf("%s: Released %s\n", __func__, #x)
#	else
# 	define LOCK(x)						pthread_mutex_lock(&(x))
# 	define UNLOCK(x)					pthread_mutex_unlock(&(x)); sched_yield()
#	endif

#endif
int _GetTickCount(void);

#endif

