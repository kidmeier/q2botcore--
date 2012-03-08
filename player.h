/*
 *  player.h
 *  prombot
 *
 *  Created by Michael Reid on 13/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __player_H__
#define __player_H__

#include <string>

#include "gamestate.h"
#include "entity.h"
#include "entityType.h"

class Player : public Entity {

public:

	enum Gender_e {
		Male,
		Female,
		Cyborg
	};
	
	class Type : public EntityType {
		
		Gender_e gender;
		
	public:
			
		Type( std::string modelString, Gender_e _gender, std::string _name )
			: EntityType(EntityType::Player, modelString), gender(_gender) {
				
			}
		
		::Player* newInstance( const entity_t& e ) {
			return new ::Player( this->gender, e );
		}
		
	};
	
	Player( Gender_e _gender, const entity_t& e )
		: Entity(e), gender(_gender) {
	
		char playerName[65];
			
		qbGetPlayerName( Gamestate::instance().indexOf(e), playerName );
		this->name = playerName;
	}
	
	void update( const entity_t& e );
	void update( const player_t& e );

	Gender_e gender;
	std::string name;
	
};

#endif

