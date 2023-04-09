/*
 * dgkml_entity.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_KML_DGKML_ENTITY_H_
#define DEREGLOBUS_KML_DGKML_ENTITY_H_

#include "kml_feature.h"

#include "dereGlobus.h"


// Feature_Type is the type of KML Feature the entity should be generated as.
template<typename KML_Feature_Type>
class DGKML_Entity : public KML_Feature_Type{

public:
	DGKML_Entity(const dg::Texts_Map& texts_map, const dg::Entity& entity)
		: KML_Feature_Type()
	{
		if(entity.get_name_id().has_value()){
			KML_Feature_Type::set_name(texts_map.get_inPrimaryLanguage_defaultToId(*entity.get_name_id()));
		}else{
			KML_Feature_Type::set_name(entity.get_id() + ".name");
		}
	}
};



#endif /* DEREGLOBUS_KML_DGKML_ENTITY_H_ */
