/*
 *  client.cpp
 *  prombot
 *
 *  Created by Michael Reid on 21/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "client.h"

void Client::update( const player_t& pl ) {
	
	this->origin = pl.origin;
	this->sureOrigin = pl.sure_origin;
	this->velocity = pl.velocity;
	this->direction.yaw = pl.angles[0];
	this->direction.pitch = pl.angles[1];
	this->direction.roll = 0;
	
	this->health = pl.health;
	this->ammo = pl.ammo;
	this->armor = pl.armor;
	
	this->timer = pl.timer;
	this->frags = pl.frags;
}
