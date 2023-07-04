/*
 * dgkml_town.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_KML_DGKML_TOWN_H_
#define DEREGLOBUS_KML_DGKML_TOWN_H_

#include "dgkml_settlement.h"

#include "kml_iconStyle.h"

class DGKML_Town : public DGKML_Settlement {

public:

	DGKML_Town(const dg::Texts_Map& texts_map, dg::Texts_Map& texts_map_local, const dg::Town& town)
		:DGKML_Settlement(texts_map, texts_map_local, town)
	{

		//TODO: Option for relative path in dereglobus for icon
		std::string icon_href;
		auto population_recent = town.get_population_recent();

		if(!population_recent.has_value() || *population_recent <= 500){
			icon_href = "http://www.dereglobus.orkenspalter.de/public/DereGlobus/Staedte/media/icons/staedte/Dorf.png";
		}else if(*population_recent <= 1000){
			icon_href = "http://www.dereglobus.orkenspalter.de/public/DereGlobus/Staedte/media/icons/staedte/Kleinstadt.png";
		}else if(*population_recent <= 5000){
			icon_href = "http://www.dereglobus.orkenspalter.de/public/DereGlobus/Staedte/media/icons/staedte/Stadt.png";
		}else if(*population_recent <= 20000){
			icon_href = "http://www.dereglobus.orkenspalter.de/public/DereGlobus/Staedte/media/icons/staedte/Grossstadt.png";
		}else{
			icon_href = "http://www.dereglobus.orkenspalter.de/public/DereGlobus/Staedte/media/icons/staedte/Metropole.png";
		}

		auto style_town = kml::StyleMap();

		auto style_town_normal = kml::Style();
		style_town_normal.set_iconStyle(kml::IconStyle(icon_href, 1.0f, 0.0f));
		style_town_normal.set_labelStyle(kml::LabelStyle(0));

		auto style_town_highlight = kml::Style();
		style_town_highlight.set_iconStyle(kml::IconStyle(icon_href, 1.5f, 0.0f));
		style_town_highlight.set_labelStyle(kml::LabelStyle(1.0f));

		style_town.set_normal_internal(std::move(style_town_normal));
		style_town.set_highlight_internal(std::move(style_town_highlight));

		set_styleSelector(style_town);
	}

};

#endif /* DEREGLOBUS_KML_DGKML_TOWN_H_ */
