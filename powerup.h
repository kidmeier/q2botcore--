/*
 *  powerup.h
 *  prombot
 *
 *  Created by Michael Reid on 15/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __powerup_H__
#define __powerup_H__

#include "item.h"
#include "entityType.h"

class Powerup : public Item {
	
public:
	enum PowerupType_e {
		Adrenaline,
		Bandolier,
		Breather,
		EnvironmentSuit,
		Invulnerability,
		Quad,
		Silencer
	};
	
	class Type : public EntityType {
		
		PowerupType_e pupType;
		
	public:
		
		Type( std::string modelString, PowerupType_e _pupType ) 
		: EntityType(EntityType::Powerup, modelString), pupType(_pupType) {
			
		}
		
		::Powerup* newInstance( const entity_t& e ) {
			return new ::Powerup( this->pupType, e );
		}
		
	};
	
	Powerup( PowerupType_e pupType, const entity_t& e ) 
	: Item(Item::Powerup, e), kind(pupType) {
		
	}

	PowerupType_e kind;
	
	void update( const entity_t& e );
	
};

#endif
