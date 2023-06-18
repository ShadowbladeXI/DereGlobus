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
	const dg::Texts_Map* texts_map_ref;//tmporary store a pointer to the global texts map
	dg::Texts_Map* texts_map_local_ref;//tmporary store a pointer to the texts map of a single file

public:

	std::unique_ptr<kml::Feature> make_typeDeduction(const dg::Texts_Map& texts_map, dg::Texts_Map& texts_map_local, const dg::Entity& entity){
		texts_map_ref = &texts_map;
		texts_map_local_ref = &texts_map_local;

		return std::move(dg::DereGlobus_ExternalFactory<kml::Feature>::make_typeDeduction(entity));
	}


	std::unique_ptr<kml::Feature> make_fixedType(const dg::Entity& entity) const override{
		throw "Entity can not be translated to kml";
	}

	std::unique_ptr<kml::Feature> make_fixedType(const dg::Location& location) const override{

		return std::make_unique<DGKML_Location>(*texts_map_ref, *texts_map_local_ref, location);

	}

	/*std::unique_ptr<kml::Feature> make_fixedType(const dg::Settlement& settlement) const override{
		return make_fixedType(static_cast<std::remove_reference_t<decltype(settlement)>::DereGlobusParentType>(settlement));
	}*/

	std::unique_ptr<kml::Feature> make_fixedType(const dg::Town& town) const override{
		return std::make_unique<DGKML_Town>(*texts_map_ref, *texts_map_local_ref, town);
	}

};



#endif /* DEREGLOBUS_KML_DGKML_FACTORY_H_ */
