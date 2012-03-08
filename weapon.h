/*
 *  weapon.h
 *  prombot
 *
 *  Created by Michael Reid on 15/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __weapon_H__
#define __weapon_H__

#include <string>

#include "entityType.h"
#include "item.h"

class Weapon : public Item {
	
public:

	enum WeaponType_e {
		
		Blaster,
		Shotgun,
		SuperShotgun,
		MachineGun,
		ChainGun,
		Grenades,
		GrenadeLauncher,
		RocketLauncher,
		Hyperblaster,
		Railgun,
		BFG
	};
	
	class Type : public EntityType {
		
		WeaponType_e weaponType;
		
	public:
		
		Type( std::string modelString, WeaponType_e _weaponType ) 
		: EntityType(EntityType::Projectile, modelString), weaponType(_weaponType) {
			
		}
		
		::Weapon* newInstance( const entity_t& e ) {
			return new ::Weapon( this->weaponType, e );
		}
		
	};
	
	Weapon( WeaponType_e _weaponType, const entity_t& e )
	: Item(Item::Weapon,e), weaponType(_weaponType) {
		
	}
	
	WeaponType_e weaponType;
	
	void update( const entity_t& e );
};

#endif

