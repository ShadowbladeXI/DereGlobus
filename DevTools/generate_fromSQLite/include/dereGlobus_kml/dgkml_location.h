/*
 * dgkml_location.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_KML_DGKML_LOCATION_H_
#define DEREGLOBUS_KML_DGKML_LOCATION_H_

class DGKML_Location : public DGKML_Entity<kml::Placemark> {

public:

	DGKML_Location(const dg::Texts_Map& texts_map, dg::Texts_Map& texts_map_local, const dg::Location& location)
		:DGKML_Entity(texts_map, texts_map_local, location)
	{
		if(location.get_coordinates().has_value()){
			auto coordinates = *location.get_coordinates();

			double n_deg = std::get<0>(coordinates);
			double e_deg = std::get<1>(coordinates);
			double altitude_m = std::get<2>(coordinates);

			set_geometry(std::make_unique<kml::Point>(kml::Coordinates(e_deg, n_deg, altitude_m)));
		}
	}

};



#endif /* DEREGLOBUS_KML_DGKML_LOCATION_H_ */
