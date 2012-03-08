/*
 *  edict.h
 *  prombot
 *
 *  Created by Michael Reid on 21/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __edict_H__
#define __edict_H__

#include "vector.h"

struct Edict {
	
	Edict() {
		
	}
	virtual ~Edict() {
		
	}
	
	// Basic state
	struct {
		float yaw;
		float pitch;
		float roll;
	} direction;
	
	vector origin;
	vector velocity;	
};

#endif
