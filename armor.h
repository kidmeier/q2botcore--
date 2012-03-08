/*
 *  armor.h
 *  prombot
 *
 *  Created by Michael Reid on 14/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __armor_H__
#define __armor_H__

#include "entityType.h"
#include "item.h"

class Armor : public Item {
	
public:
	
	enum ArmorType_e {
		Jacket,
		Combat,
		Body,
		Shard,
		PowerScreen,
		PowerShield
	};
	
	class Type : public EntityType {
		
		ArmorType_e armorType;
		int amount;
		
	public:
			
			Type( std::string modelString, ArmorType_e _armorType, int _amount ) 
			: EntityType(EntityType::Armor, modelString), armorType(_armorType) {
				
			}
		
		::Armor* newInstance( const entity_t& e ) {
			return new ::Armor( this->armorType, this->amount, e );
		}
		
	};
	
	Armor( ArmorType_e _armorType, int _amount, const entity_t& e )
	: Item(Item::Armor, e), armorType(_armorType), amount(_amount)  {
		
	}
	
	ArmorType_e armorType;
	int amount;
	
	void update( const entity_t& e );
};

#endif
