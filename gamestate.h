/*
 *  gamestate.h
 *  prombot
 *
 *  Created by Michael Reid on 30/06/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __gamestate_H__
#define __gamestate_H__

#include <list>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "q2bot.h"

#include "client.h"
#include "vector.h"

class Entity;
class Player;
class Item;
	class Weapon;
	class Ammo;
	class Powerup;

	class Health;
	class Armor;
class Projectile;


class Gamestate {

	// its a singleton
	Gamestate() : pollLevelTime(-POLL_LEVEL_TIMEOUT) {
		this->entities.resize(MaxEntities);
	}
	
	void levelChanged( const std::string& );
	
public :
	
	enum {
		MaxEntities = 1024,
		MaxModels = 256,
		MaxSounds = 256,
		MaxClients = 256,
		MaxItems = 256,
		
		POLL_LEVEL_TIMEOUT = 10000,
	};
	
	static std::map<int,std::string> modelStrings;
	
	static Gamestate& instance();
	~Gamestate() {
	
	}

	// Gamestate
	long pollLevelTime;
	
	gamestate_t* gs;
	std::string mapName;

	Client client;
	int clientEntity;
	
	// Entity lists
	std::vector<Entity*> entities;
	std::map<std::string,Player*> players;
	std::list<Item*> items;
	// Sub-categorized items
		std::list<Ammo*> ammo;
		std::list<Armor*> armor;
		std::list<Weapon*> weapons;
		std::list<Health*> health;
		std::list<Powerup*> powerups;
	std::list<Projectile*> projectiles;
	
	int indexOf( const entity_t& e );
	// Update our metadata from the raw q2bot server state
	void update(gamestate_t& gs);
};

#endif
