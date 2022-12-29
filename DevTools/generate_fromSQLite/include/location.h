/*
 * location.h
 *
 *  Created on: Dec 26, 2022
 *      Author: ShadowbladeXI
 */

#ifndef LOCATION_H_
#define LOCATION_H_

#include <string>
#include <array>
#include <tuple>
#include <optional>

#include "sqlite3.h"
#include "sqlite3_tableRead_helper.h"

#include "../includes/rapidxml-1.13/rapidxml.hpp"

#include "math/tuple_math.h"

//Represents an object with a defined single location
class Location{
private:
	std::string id;
	std::array<double, 3> location;

public:

	//Class for the information for the SQL request to generate a new object
	class SQL_RequestList : public SQLite_Helper_RequestList{
	private:
		typedef SQLite_Helper_RequestList Parent;//Define parent for easier copying of boilerplate

	private:
		//Information in which table and under which name the attributes for the individual class members can be found
		constexpr static auto id_attribute = SQLite3_Helper_Attribute<std::string>("location", "id");
		constexpr static auto nameID_attribute = SQLite3_Helper_Attribute<std::string>("location", "name_id");
		constexpr static auto locationNDeg_attribute = SQLite3_Helper_Attribute<double>("location", "location_n_deg");
		constexpr static auto locationEDeg_attribute = SQLite3_Helper_Attribute<double>("location", "location_e_deg");

		constexpr static auto attributes_part = std::make_tuple(id_attribute, nameID_attribute, locationNDeg_attribute, locationEDeg_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function

	public:
		std::optional<std::string> id;
		std::optional<std::string> name_id;
		std::optional<double> location_n_deg;
		std::optional<double> location_e_deg;

	protected:
		auto get_attributeValues_part(){
			return std::tie(id, name_id, location_n_deg, location_e_deg);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
		}

	public:
		/*
		 * Boilerplate interface for creating new instances, can be copied without modification for children/unrelated classes
		 */
		static auto get_attributes(){
			return std::tuple_cat(Parent::get_attributes(), attributes_part);
		}

		static constexpr std::size_t attributeIndex_start = std::tuple_size_v<decltype(Parent::get_attributes())>;
		static constexpr std::size_t attributeIndex_end = attributeIndex_start + std::tuple_size_v<decltype(attributes_part)>;
		template<typename AttributeValueTuple_Type>
		SQL_RequestList(const AttributeValueTuple_Type& attributeValues)
			:Parent(attributeValues)
			//,attributeValues_part(get_tuplePart<attributeIndex_start, attributeIndex_end>(attributeValues))
		{
			get_attributeValues_part() = get_tuplePart<attributeIndex_start, attributeIndex_end>(attributeValues);
		}
	};

public:

	Location(std::string id, std::array<double, 3> location)
		:id(id)
		,location(location)
	{}

	//Uses data from an SQL Request to create an object.
	//Returns an empty optional if construction failed
	static std::optional<Location> make_fromSQLRequest(const SQL_RequestList& requestList){
		assert(requestList.id.has_value());//ID should always be defined

		if(requestList.location_e_deg.has_value() && requestList.location_n_deg.has_value()){
			return Location(*(requestList.id), {*(requestList.location_e_deg), *(requestList.location_n_deg), 0.0});//0.0 as default height
		}else{
			//Cannot add a location at an undefined position in DereGlobus
			std::cout << "undefined location!" << std::endl;
			return std::nullopt;
		}
	}

public:
	/*
	 * Functionality for .kml file creation
	 */

	//Function to create a kml node for this entity inside the document doc
	template<typename Ch=char>
	rapidxml::xml_node<Ch>* build_kmlNode(rapidxml::xml_document<Ch>& doc) const{
		auto* node_entity = doc.allocate_node(rapidxml::node_element, "Placemark");

		auto* node_name = doc.allocate_node(rapidxml::node_element, "name", id.c_str());
		node_entity->append_node(node_name);

		auto* node_point = doc.allocate_node(rapidxml::node_element, "Point");
		node_entity->append_node(node_point);
		auto* coordinates_cstring = doc.allocate_string((std::to_string(std::get<0>(location)) + ", " + std::to_string(std::get<1>(location)) + ", " + std::to_string(std::get<2>(location))).c_str());
		auto* node_point_coordinates = doc.allocate_node(rapidxml::node_element, "coordinates", coordinates_cstring);
		node_point->append_node(node_point_coordinates);

		return node_entity;
	}
};



#endif /* LOCATION_H_ */
