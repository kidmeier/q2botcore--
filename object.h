/*
 *  object.h
 *  prombot
 *
 *  Created by Michael Reid on 21/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __object_H__
#define __object_H__

#include <string>

#include "entity.h"
#include "entityType.h"

class Object : Entity {

public:
	class Type : public EntityType {
	
	public:
		
		Type( std::string modelString )
		: EntityType(EntityType::Object, modelString) {
			
		}
		
		::Object* newInstance( const entity_t& e ) {
			return new ::Object( e );
		}
		
	};
	
	Object( const entity_t& e )
	: Entity(e) {
			
	}
	
	void update( const entity_t& e );
};

#endif

