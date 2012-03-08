/*
 *  entities.cpp
 *  prombot
 *
 *  Created by Michael Reid on 21/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ammo.h"
#include "armor.h"
#include "health.h"
#include "object.h"
#include "player.h"
#include "powerup.h"
#include "projectile.h"
#include "weapon.h"
#include "entities.h"

void registerEntityTypes() {

	new Object::Type("deadbods/dude");

	new Ammo::Type("items/ammo/bullets/medium", Ammo::Bullets, 20);
	new Ammo::Type("items/ammo/cells/medium", Ammo::Cells, 20);
	new Ammo::Type("items/ammo/grenades/medium", Ammo::Grenades, 5);
	new Ammo::Type("items/ammo/rockets/medium", Ammo::Rockets, 5);
	new Ammo::Type("items/ammo/shells/medium", Ammo::Shells, 10);
	new Ammo::Type("items/ammo/slugs/medium", Ammo::Slugs, 10);
	//Ammo::Type("items/ammo/mines", Ammo::Min
	//Ammo::Type("items/ammo/nuke",

	new Armor::Type("items/armor/body", Armor::Body, 150);
	new Armor::Type("items/armor/combat", Armor::Combat, 100);
	//Armor::Type("items/armor/effect", Ar
	new Armor::Type("items/armor/jacket", Armor::Jacket, 50);
	new Armor::Type("items/armor/screen", Armor::PowerScreen, 250);
	new Armor::Type("items/armor/shard", Armor::Shard, 2);
	new Armor::Type("items/armor/shield", Armor::PowerShield, 100);

	new Powerup::Type("items/adrenal", Powerup::Adrenaline );
	new Powerup::Type("items/band", Powerup::Bandolier);
	new Powerup::Type("items/breather", Powerup::Breather);
	new Powerup::Type("items/enviro", Powerup::EnvironmentSuit);
	new Powerup::Type("items/invulner", Powerup::Invulnerability);
	new Powerup::Type("items/quaddama", Powerup::Quad);
	new Powerup::Type("items/silencer", Powerup::Silencer);
	//::Type("items/c_head"

	new Health::Type("items/healing/large", 50);
	new Health::Type("items/healing/medium", 25);
	new Health::Type("items/healing/stimpack", 10);
	new Health::Type("items/mega_h", 100);

	new Object::Type("items/keys/data_cd");
	new Object::Type("items/keys/key");
	new Object::Type("items/keys/pass");
	new Object::Type("items/keys/power");
	new Object::Type("items/keys/pyramid");
	new Object::Type("items/keys/red_key");
	new Object::Type("items/keys/spinner");
	new Object::Type("items/keys/target");
	//::Type("items/pack"
	new Object::Type("objects/banner");
	new Object::Type("objects/barrels");
	new Object::Type("objects/black");
	new Object::Type("objects/bomb");
	new Object::Type("objects/debris1");
	new Object::Type("objects/debris2");
	new Object::Type("objects/debris3");
	new Object::Type("objects/dmspot");
	new Object::Type("objects/explode");
	new Object::Type("objects/flash");
	new Object::Type("objects/gibs/arm");
	new Object::Type("objects/gibs/bone");
	new Object::Type("objects/gibs/bone2");
	new Object::Type("objects/gibs/chest");
	new Object::Type("objects/gibs/gear");
	new Object::Type("objects/gibs/head");
	new Object::Type("objects/gibs/head2");
	new Object::Type("objects/gibs/leg");
	new Object::Type("objects/gibs/skull");
	new Object::Type("objects/gibs/sm_meat");
	new Object::Type("objects/gibs/sm_metal");
	new Object::Type("objects/minelite/light1");
	new Object::Type("objects/minelite/light2");
	new Object::Type("objects/r_explode");
	new Object::Type("objects/satellite");
	new Object::Type("objects/smoke");
	new Object::Type("ships/bigviper");
	new Object::Type("ships/strogg1");
	new Object::Type("ships/viper");
					 
	new Weapon::Type("weapons/g_bfg", Weapon::BFG);
	new Weapon::Type("weapons/g_blast", Weapon::Blaster);
	new Weapon::Type("weapons/g_chain", Weapon::ChainGun);
	//Weapon::Type("weapons/g_disint", Weapon::);
	//Weapon::Type("weapons/g_flareg", Weapon::);
	new Weapon::Type("weapons/g_hyperb", Weapon::Hyperblaster);
	new Weapon::Type("weapons/g_launch", Weapon::GrenadeLauncher);
	new Weapon::Type("weapons/g_machn", Weapon::MachineGun);
	new Weapon::Type("weapons/g_rail", Weapon::Railgun);
	new Weapon::Type("weapons/g_rocket", Weapon::RocketLauncher);
	new Weapon::Type("weapons/g_shotg", Weapon::Shotgun);
	new Weapon::Type("weapons/g_shotg2", Weapon::SuperShotgun);
	new Weapon::Type("weapons/v_bfg", Weapon::BFG);
	new Weapon::Type("weapons/v_blast", Weapon::Blaster);
	new Weapon::Type("weapons/v_chain", Weapon::ChainGun);
	//Weapon::Type("weapons/v_disint", Weapon::);
	//Weapon::Type("weapons/v_flareg", Weapon::);
	new Weapon::Type("weapons/v_handgr", Weapon::Grenades);
	new Weapon::Type("weapons/v_hyperbv", Weapon::Hyperblaster);
	new Weapon::Type("weapons/v_launch", Weapon::GrenadeLauncher);
	new Weapon::Type("weapons/v_machn", Weapon::MachineGun);
	new Weapon::Type("weapons/v_rail", Weapon::Railgun);
	new Weapon::Type("weapons/v_rocket", Weapon::RocketLauncher);
	new Weapon::Type("weapons/v_shotg", Weapon::Shotgun);
	new Weapon::Type("weapons/v_shotg2", Weapon::SuperShotgun);
	
	new Projectile::Type("objects/grenade", Projectile::Grenade);
	new Projectile::Type("objects/grenade2", Projectile::LaunchedGrenade);
	new Projectile::Type("objects/laser", Projectile::Blaster);
	new Projectile::Type("objects/rocket", Projectile::Rocket);
					 
	 /*
	  monsters/berserk
	  monsters/bitch
	  monsters/boss1
	  monsters/boss2
	  monsters/boss3/jorg
	  monsters/boss3/rider
	  monsters/brain
	  monsters/commandr/head
	  monsters/commandr
	  monsters/flipper
	  monsters/float
	  monsters/flyer
	  monsters/gladiatr
	  monsters/gunner
	  monsters/hover
	  monsters/infantry
	  monsters/insane
	  monsters/medic
	  monsters/mutant
	  monsters/parasite/segment
	  monsters/parasite/tip
	  monsters/parasite
	  monsters/soldier
	  monsters/tank
	  */
}