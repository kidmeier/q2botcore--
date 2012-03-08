/*
 *  health.h
 *  prombot
 *
 *  Created by Michael Reid on 14/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __health_H__
#define __health_H__

#include "entityType.h"
#include "item.h"

class Health : public Item {
	
public:
	
	class Type : public EntityType {
		
		int amount;
		
	public:
			
			Type( std::string modelString, int _amount ) 
			: EntityType(EntityType::Health, modelString) {

			}
		
		::Health* newInstance( const entity_t& e ) {
			return new ::Health( this->amount, e );
		}
		
	};
	
	Health( int _amount, const entity_t& e ) 
	: Item(Item::Health, e), amount(_amount) {
		
	}
	void update( const entity_t& e );
	
	int amount;
};

#endif
