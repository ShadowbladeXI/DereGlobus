/*
 * dgkml_settlement.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_KML_DGKML_SETTLEMENT_H_
#define DEREGLOBUS_KML_DGKML_SETTLEMENT_H_

#include "dgkml_location.h"

#include <cmath>

class DGKML_Settlement : public DGKML_Location {

public:

	DGKML_Settlement(const dg::Texts_Map& texts_map, dg::Texts_Map& texts_map_local, const dg::Settlement& settlement)
		:DGKML_Location(texts_map, texts_map_local, settlement)
	{
		if(settlement.get_coordinates().has_value()){

			//Calculate size
			unsigned int pop_norm;
			unsigned int pop_norm_min = 100;
			if(!settlement.get_population().has_value() || *settlement.get_population() < pop_norm_min){
				pop_norm = pop_norm_min;
			}else{
				pop_norm = *settlement.get_population();
			}

			float a = 1e4f * std::exp(-2.1);
			float b = 0.7;
			float size_m2 = a*std::pow(pop_norm, b);
			float size_m = std::sqrt(size_m2);

			float size_deg = size_m/4e5f;

			float e_deg = settlement.get_coordinates()->at(0);
			float n_deg = settlement.get_coordinates()->at(1);

			float minAlt = 0.0f;
			float maxAlt = 0.0f;

			//Set LOD
			float lod_minPx = 0.1;
			float lod_maxPx = -1;
			float lod_minFade_extend = 0;
			float lod_maxFade_extend = 0;

			//Combine
			set_region(kml::Region(n_deg + size_deg/2,  n_deg - size_deg/2, e_deg + size_deg/2,  e_deg - size_deg/2, minAlt, maxAlt, lod_minPx, lod_maxPx, lod_minFade_extend, lod_maxFade_extend));
		}
	}

};



#endif /* DEREGLOBUS_KML_DGKML_SETTLEMENT_H_ */
