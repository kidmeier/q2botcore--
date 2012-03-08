/*
 *  entityType.cpp
 *  prombot
 *
 *  Created by Michael Reid on 13/07/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "entityType.h"
#include "entity.h"

std::map<std::string,EntityType*> EntityType::typeMap;
EntityType EntityType::unknownType(EntityType::Unknown, "unknown");

EntityType& EntityType::classify( const entity_t& e ) {
	
	char str[65];
	qbGetModelString(e.modelindex,str);
	
	std::string modelString = str;
	
	std::string::size_type size = modelString.find("models/");
	
	if( modelString.find("models/") == std::string::npos )
		return EntityType::unknownType;
	
	modelString.replace( modelString.find("models/"), std::string("models/").length(), "" );
	modelString.replace( modelString.find("/tris.md2"), std::string("/tris.md2").length(), "");
	
	EntityType* type = typeMap[ modelString ];
	if( !type )
		return EntityType::unknownType;
	
	return *type;
}

Entity* EntityType::newInstance( const entity_t& e ) {
	
	return new Entity(e);
}
