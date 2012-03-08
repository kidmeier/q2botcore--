/*
 *  entity.cpp
 *  prombot
 *
 *  Created by Michael Reid on 13/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "gamestate.h"
#include "entity.h"

Entity::Entity( const entity_t& e ) : type(EntityType::classify(e)) {

	*this = e;
}

void Entity::update( const entity_t& e ) {
	
	this->origin = e.origin;
	this->velocity = e.velocity;
	
	vector angles = vector::to_angles( origin );
	this->direction.pitch = angles[0];
	this->direction.yaw = angles[1];
	this->direction.roll = angles[2];
	
	this->modelindex = e.modelindex;
	this->modelindex2 = e.modelindex2;
	this->modelindex3 = e.modelindex3;
	this->modelindex4 = e.modelindex4;
	
	this->frame = e.framenum;
	this->renderfx = e.renderfx;
	
	this->visible = e.updated;
}

Entity& Entity::operator=( const entity_t& e ) {

	this->type = EntityType::classify(e);
	this->update(e);
	
	return *this;
}

Entity& Entity::operator=( const Entity& e ) {

	this->type = e.type;
	this->direction = e.direction;
	this->origin = e.origin;
	this->velocity = e.velocity;
	
	this->modelindex = e.modelindex;
	this->modelindex2 = e.modelindex2;
	this->modelindex3 = e.modelindex3;
	this->modelindex4 = e.modelindex4;
	
	this->frame = e.frame;
	this->renderfx = e.renderfx;
	this->visible = e.visible;
	
	return *this;
}
