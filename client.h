/*
 *  client.h
 *  prombot
 *
 *  Created by Michael Reid on 21/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __client_H__
#define __client_H__

#include "edict.h"

#include "ammo.h"
#include "armor.h"
#include "weapon.h"

#include "vector.h"
#include "q2bot.h"

struct Client : public Edict {

	void update( const player_t& pl );

	vector sureOrigin;
	
	int health;
	
	Weapon::WeaponType_e weapon;
	Ammo::AmmoType_e ammoType;
	int ammo;
	
	Armor::ArmorType_e armorType;
	int armor;
	
	int timer;
	int frags;
};

#endif
