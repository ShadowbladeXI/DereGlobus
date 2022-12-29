/*
 * settlement.h
 *
 *  Created on: Dec 25, 2022
 *      Author: ShadowbladeXI
 */

#ifndef SETTLEMENT_H_
#define SETTLEMENT_H_

#include <string>
#include <array>

#include "location.h"

#include "sqlite3.h"
#include "sqlite3_tableRead_helper.h"

class Settlement : public Location{
private:
	std::optional<unsigned int> population;//No value means that population is unknown/not yet in the database

public:

	//Class for the information for the SQL request to generate a new object
	class SQL_RequestList : public Location::SQL_RequestList{
	private:
		typedef Location::SQL_RequestList Parent;//Define parent for easier copying of boilerplate

	public:
		//Information in which table and under which name the attributes for the individual class members can be found
		constexpr static auto population_attribute = SQLite3_Helper_Attribute<unsigned int>("settlement", "population");

		constexpr static auto attributes_part = std::make_tuple(population_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function


	public:
		std::optional<unsigned int> population;

	protected:
		auto get_attributeValues_part(){
			return std::tie(population);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
		}

	public:
		/*
		 * Boilerplate interface for creating new instances, can be copied without modification for children
		 */
		static auto get_attributes(){
			return std::tuple_cat(Parent::get_attributes(), attributes_part);
		}

		static constexpr std::size_t attributeIndex_start = std::tuple_size_v<decltype(Parent::get_attributes())>;
		static constexpr std::size_t attributeIndex_end = attributeIndex_start + std::tuple_size_v<decltype(attributes_part)>;
		template<typename AttributeValueTuple_Type>
		SQL_RequestList(const AttributeValueTuple_Type& attributeValues)
			:Parent(attributeValues)
		{
			get_attributeValues_part() = get_tuplePart<attributeIndex_start, attributeIndex_end>(attributeValues);
		}
	};

public:
	Settlement(Location location, std::optional<unsigned int> population)
		:Location(location)
		,population(population)
	{}

	//Uses data from an SQL Request to create an object.
	//Returns an empty optional if construction failed
	static std::optional<Settlement> make_fromSQLRequest(const SQL_RequestList& requestList){
		auto location_opt = Location::make_fromSQLRequest(requestList);
		if(location_opt.has_value()){
			return Settlement(*location_opt, requestList.population);
		}else{
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
		auto* node_entity = Location::build_kmlNode(doc);

		//TODO: add settlement specific information

		return node_entity;
	}
};



#endif /* SETTLEMENT_H_ */
