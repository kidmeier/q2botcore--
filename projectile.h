/*
 *  projectile.h
 *  prombot
 *
 *  Created by Michael Reid on 15/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __projectile_H__
#define __projectile_H__

#include "entity.h"
#include "entityType.h"

class Projectile : public Entity {
	
public:
	
	enum ProjectileType_e {
		Grenade,
		LaunchedGrenade,
		Rocket,
		Blaster
	};
	
	class Type : public EntityType {
		
		ProjectileType_e projectileType;
		
	public:
		
		Type( std::string modelString, ProjectileType_e _projectileType ) 
		: EntityType(EntityType::Projectile, modelString), projectileType(_projectileType) {
			
		}
		
		::Projectile* newInstance( const entity_t& e ) {
			return new ::Projectile( this->projectileType, e );
		}
		
	};
	
	Projectile( ProjectileType_e _projectileType, const entity_t& e )
	: Entity(e), projectileType(_projectileType) {
		
		// Calculate impactDest, impactTime
	}
	
	ProjectileType_e projectileType;
	vector impactDest;
	int impactTime;
	
	void update( const entity_t& e );
};

#endif
