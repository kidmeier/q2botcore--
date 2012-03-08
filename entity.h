/*
 *  entity.h
 *  prombot
 *
 *  Created by Michael Reid on 13/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __entity_H__
#define __entity_H__

#include "edict.h"
#include "q2bot.h"

class EntityType;

struct Entity : public Edict {

	// Ctors
	Entity();
	Entity(const entity_t& e);
	Entity& operator= (const entity_t& e);
	Entity& operator= (const Entity& e);
	
	virtual void update( const entity_t& e );
	
	// Data
	EntityType& type;
	
	int modelindex;
	int modelindex2;
	int modelindex3;
	int modelindex4;
	
	int frame;
	int renderfx;
	
	bool visible;
};

#endif
