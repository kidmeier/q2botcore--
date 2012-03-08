/*
 *  entityType.h
 *  prombot
 *
 *  Created by Michael Reid on 13/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __entityType_H__
#define __entityType_H__

#include <cstdarg>
#include <map>
#include <string>

#include "q2bot.h"

class Entity;

class EntityType {

public :
	enum Class_e {
		Unknown,
		Player,
		Health,
		Armor,
		Weapon,
		Ammo,
		Powerup,
		Projectile,
		Object
	};
	
private:	
	static std::map<std::string,EntityType*> typeMap;
	static EntityType unknownType;
	
protected:

	EntityType( Class_e _clazz, std::string _modelString ) 
		: clazz(_clazz), modelString(_modelString) {
	
		// Record ourself in the type-map.
		typeMap[_modelString] = this;
	}
	virtual ~EntityType() {
		
	}

public:
	// This is the factory method which derived types implement
	virtual Entity* newInstance( const entity_t& e );
	static EntityType& classify( const entity_t& e );
	
	Class_e clazz;
	std::string modelString;
};
 
#endif
