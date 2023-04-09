/*
 * location.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_DG_LOCATION_H_
#define DEREGLOBUS_DG_LOCATION_H_

#include "entity.h"

namespace dg {

class Location : public Entity{
public:
	typedef Entity DereGlobusParentType;

protected:
	std::optional<std::array<double, 3>> coordinates;

public:

	//Class for the information for the SQL request to generate a new object
	class SQL_RequestList : public Entity::SQL_RequestList{
	private:
		typedef Entity::SQL_RequestList Parent;//Define parent for easier copying of boilerplate

	private:
		//Information in which table and under which name the attributes for the individual class members can be found
		constexpr static auto locationNDeg_attribute = SQLite3_Helper_Attribute<double>("location", "location_n_deg");
		constexpr static auto locationEDeg_attribute = SQLite3_Helper_Attribute<double>("location", "location_e_deg");

		constexpr static auto attributes_part = std::make_tuple(locationNDeg_attribute, locationEDeg_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function

	public:
		std::optional<double> location_n_deg;
		std::optional<double> location_e_deg;

	protected:
		auto get_attributeValues_part(){
			return std::tie(location_n_deg, location_e_deg);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
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

	Location(Entity entity, std::optional<std::array<double, 3>> coordinates)
		:Entity(entity)
		,coordinates(coordinates)
	{}

	virtual ~Location(){}

	const std::optional<std::array<double, 3>>& get_coordinates() const{
		return coordinates;
	}

	//Uses data from an SQL Request to create an object.
	//Returns an empty optional if construction failed
	static std::optional<Location> make_fromSQLRequest(const SQL_RequestList& requestList){
		auto entity_opt = Entity::make_fromSQLRequest(requestList);

		if(entity_opt.has_value()){
			std::optional<std::array<double, 3>> location;
			if(requestList.location_e_deg.has_value() && requestList.location_n_deg.has_value()){
				location = std::optional<std::array<double, 3>>({*(requestList.location_e_deg), *(requestList.location_n_deg), 0.0});
			}else{
				location = std::nullopt;
			}
			return Location(*entity_opt, location);
		}else{
			return std::nullopt;
		}
	}

	void execute_resolved(DereGlobus_DynamicEntityResolver& resolver) const override{
		resolver.do_fixedType(*this);
	}
	void execute_resolved(DereGlobus_DynamicEntityResolver& resolver) override{
		resolver.do_fixedType(*this);
	}
};

}

#endif /* DEREGLOBUS_DG_LOCATION_H_ */
