/*
 *  item.h
 *  prombot
 *
 *  Created by Michael Reid on 14/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __item_H__
#define __item_H__

#include "entity.h"

class Item : public Entity {

public:
	enum Kind_e {
		Ammo,
		Weapon,
		
		Armor,
		Health,
		Powerup
	};
	
protected:

	Item( Kind_e _kind, const entity_t& e )
		: Entity(e), kind(_kind) {
	
	}

public:

	const Kind_e kind;
	
	void update( const entity_t& e );
};

#endif

