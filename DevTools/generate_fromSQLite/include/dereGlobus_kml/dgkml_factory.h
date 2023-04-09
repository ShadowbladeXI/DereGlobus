/*
 * dgkml_factory.h
 *
 *  Created on: 8 Apr 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_KML_DGKML_FACTORY_H_
#define DEREGLOBUS_KML_DGKML_FACTORY_H_

#include <type_traits>

#include "kml.h"
#include "dereGlobus.h"

#include "dgkml_factory.h"
#include "dgkml_entities.h"

class DG_KML_Factory : private dg::DereGlobus_ExternalFactory<kml::Feature>{

private:
	const dg::Texts_Map* texts_map_ref;//tmporary store a pointer to the texts map

public:

	std::unique_ptr<kml::Feature> make_typeDeduction(const dg::Texts_Map& texts_map, const dg::Entity& entity){
		texts_map_ref = &texts_map;
		return std::move(dg::DereGlobus_ExternalFactory<kml::Feature>::make_typeDeduction(entity));
	}


	std::unique_ptr<kml::Feature> make_fixedType(const dg::Entity& entity) const override{
		throw "Entity can not be translated to kml";
	}

	std::unique_ptr<kml::Feature> make_fixedType(const dg::Location& location) const override{

		return std::make_unique<DGKML_Location>(*texts_map_ref, location);

	}

	std::unique_ptr<kml::Feature> make_fixedType(const dg::Settlement& settlement) const override{
		return make_fixedType(static_cast<std::remove_reference_t<decltype(settlement)>::DereGlobusParentType>(settlement));
	}


};



#endif /* DEREGLOBUS_KML_DGKML_FACTORY_H_ */
