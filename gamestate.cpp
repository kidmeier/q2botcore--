/*
 *  gamestate.cpp
 *  prombot
 *
 *  Created by Michael Reid on 30/06/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "gamestate.h"

#include "ammo.h"
#include "armor.h"
#include "health.h"
#include "item.h"
#include "player.h"
#include "entity.h"
#include "entityType.h"
#include "powerup.h"
#include "projectile.h"
#include "weapon.h"

std::map<int,std::string> Gamestate::modelStrings;

Gamestate& Gamestate::instance() {

	static Gamestate theInstance;
	return theInstance;
}

void Gamestate::levelChanged( const std::string& mapname ) {

	this->mapName = mapname;
}

int Gamestate::indexOf( const entity_t& e ) {
	
	if( !this->gs )
		return -1;
	
	return &e - this->gs->entities;
}

void Gamestate::update( gamestate_t& gs ) {

	// We'll need you for later
	this->gs = &gs;
	
	// Check if we changed levels
	if( this->pollLevelTime + POLL_LEVEL_TIMEOUT < qbTickCount() ) {
		
		char mapname[64];
		
		// Check if the map changed
		qbGetModelString( 1,mapname );
		if( this->mapName != mapname )
			this->levelChanged(mapname);
		
		// We don't need to poll this every frame. Take'er easy.
		this->pollLevelTime = qbTickCount();
	}
	
	this->client.update( gs.player );
	this->clientEntity = gs.player_entity;
	
	// Clear categories
	this->ammo.clear();
	this->armor.clear();
	this->health.clear();
	this->players.clear();
	this->items.clear();
	this->projectiles.clear();
	
	// Update entities
	for( int i=1; i<1024; i++ ) {
			
		entity_t* e = &gs.entities[i];
		if( !e->updated ) {
		
			// Clean up, its not visible anymore
			if( this->entities[i] ) {
			
				delete this->entities[i];
				this->entities[i] = 0;
			}
			continue;
		}

		// Translate it into our own local coord system
		e->origin[0] = e->origin[0] - client.origin[0];
		e->origin[1] = e->origin[1] - client.origin[1];
		e->origin[2] = e->origin[2] - client.origin[2];
		
		// Velocity relative to ourself
		e->velocity[0] = e->velocity[0] - client.velocity[0];
		e->velocity[1] = e->velocity[1] - client.velocity[1];
		e->velocity[2] = e->velocity[2] - client.velocity[2];
		
		// Determine what kind of entity this is and
		// add it to the appropriate collection
		EntityType& type = EntityType::classify( *e );
		Entity* entity = type.newInstance( *e );
		entity->update( *e );
		
		entities[i] = entity;
		
		// Put it in the proper bin
		switch( type.clazz ) {
		
		case EntityType::Player: {
		
			Player* player = dynamic_cast<Player*>( entity );
			this->players[ player->name ] = player;
			break;
		}
		// Put each item into its bin
		case EntityType::Ammo: {

			Ammo* ammo = dynamic_cast<Ammo*>( entity );
			this->ammo.push_back( ammo );
			this->items.push_back( ammo );
			break;
		}
		case EntityType::Armor: {
		
			Armor* armor = dynamic_cast<Armor*>( entity );
			this->armor.push_back( armor );
			this->items.push_back( armor );
			break;
		}
		case EntityType::Weapon: {
		
			Weapon* weapon = dynamic_cast<Weapon*>( entity );
			this->weapons.push_back(weapon);
			this->items.push_back(weapon);
			break;
		}
		case EntityType::Health: {
		
			Health* health = dynamic_cast<Health*>( entity );
			this->health.push_back(health);
			this->items.push_back(health);
			break;
		}
		case EntityType::Powerup: {
		
			Powerup* pup = dynamic_cast<Powerup*>( entity );
			this->powerups.push_back(pup);
			this->items.push_back(pup);
			break;
		}
		case EntityType::Projectile: {
		
			Projectile* p = dynamic_cast<Projectile*>( entity );
			this->projectiles.push_back(p);
			break;
		}
		default:
			// you know...
			break;
		}
	}
}
