/*
 * entity.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_DG_ENTITY_H_
#define DEREGLOBUS_DG_ENTITY_H_

#include <string>
#include <optional>
#include <tuple>

#include <iostream>//TODO: Remove

#include "../sqlite3_tableRead_helper.h"

#include "../dg_dynamicEntityResolver.h"

namespace dg{

class Entity{
protected:
	std::string id;
	std::optional<std::string> name_id;
	std::optional<std::string> description_id;

public:
	//Class for the information for the SQL request to generate a new object
	class SQL_RequestList : public SQLite_Helper_RequestList{
	private:
		typedef SQLite_Helper_RequestList Parent;//Define parent for easier copying of boilerplate

	private:
		//Information in which table and under which name the attributes for the individual class members can be found
		constexpr static auto id_attribute = SQLite3_Helper_Attribute<std::string>("entity", "id");
		constexpr static auto nameID_attribute = SQLite3_Helper_Attribute<std::string>("entity", "name_id");
		constexpr static auto descriptionID_attribute = SQLite3_Helper_Attribute<std::string>("entity", "description_short_htmlText_id");

		constexpr static auto attributes_part = std::make_tuple(id_attribute, nameID_attribute, descriptionID_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function

	public:
		std::optional<std::string> id;
		std::optional<std::string> name_id;
		std::optional<std::string> description_id;

	protected:
		auto get_attributeValues_part(){
			return std::tie(id, name_id, description_id);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
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
	Entity(std::string id, std::optional<std::string> name_id, std::optional<std::string> description_id)
		:id(id)
		,name_id(name_id)
		,description_id(description_id)
	{}

	virtual ~Entity(){}

	//Uses data from an SQL Request to create an object.
	//Returns an empty optional if construction failed //TODO: Change to use exceptions instead
	static std::optional<Entity> make_fromSQLRequest(const SQL_RequestList& requestList){
		assert(requestList.id.has_value());//ID should always be defined

		return Entity(*(requestList.id), requestList.name_id, requestList.description_id);
	}

	const std::string& get_id() const{
		return id;
	}

	const std::optional<std::string>& get_name_id() const{
		return name_id;
	}

	//virtual void execute_resolved(DereGlobus_DynamicEntityResolver& resolver) const = 0;
	//virtual void execute_resolved(DereGlobus_DynamicEntityResolver& resolver) = 0;

	virtual void execute_resolved(DereGlobus_DynamicEntityResolver& resolver) const{
		resolver.do_fixedType(*this);
	}
	virtual void execute_resolved(DereGlobus_DynamicEntityResolver& resolver){
		resolver.do_fixedType(*this);
	}
};

}

#endif /* DEREGLOBUS_DG_ENTITY_H_ */
