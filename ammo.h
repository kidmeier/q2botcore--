/*
 *  ammo.h
 *  prombot
 *
 *  Created by Michael Reid on 15/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __ammo_H__
#define __ammo_H__

#include "item.h"
#include "entityType.h"

class Ammo : public Item {
	
public:
	
	enum AmmoType_e {
		Shells,
		Bullets,
		Grenades,
		Rockets,
		Slugs,
		Cells
	};
	
	class Type : public EntityType {
	
		AmmoType_e ammoType;
		int amount;
		
	public:
		
		Type( std::string modelString, AmmoType_e _ammoType, int _amount ) 
			: EntityType(EntityType::Ammo, modelString), ammoType(_ammoType) {
			
		}
		
		::Ammo* newInstance( const entity_t& e ) {
			return new ::Ammo( this->ammoType, this->amount, e );
		}
		
	};
	
	Ammo( AmmoType_e _ammoType, int _amount, const entity_t& e ) 
	: Item(Item::Ammo, e), ammoType(_ammoType), amount(_amount) {
		
	}
	
	AmmoType_e ammoType;
	int amount;
	
	void update( const entity_t& e );
};

#endif

