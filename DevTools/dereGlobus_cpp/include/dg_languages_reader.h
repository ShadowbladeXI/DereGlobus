/*
 * dg_languages_reader.h
 *
 *  Created on: 9 Apr 2023
 *      Author: Alex
 */

#include "sqlite3.h"

#include "../include/sqlite3_tableRead_helper.h"

#ifndef INCLUDE_DG_LANGUAGES_READER_H_
#define INCLUDE_DG_LANGUAGES_READER_H_

namespace dg{

//Class for reading all languages //TODO: move to other location
class SQLite_Helper_RequestList_Language : SQLite_Helper_RequestList{
private:
	typedef SQLite_Helper_RequestList Parent;//Define parent for easier copying of boilerplate

private:
	constexpr static auto id_attribute = SQLite3_Helper_Attribute<std::string>("language", "language_id");
	constexpr static auto attributes_part = std::make_tuple(id_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function

public:
	std::optional<std::string> id;

protected:
	auto get_attributeValues_part(){
		return std::tie(id);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
	}

public:
	static auto get_attributes(){
		return std::tuple_cat(Parent::get_attributes(), attributes_part);
	}

	static constexpr std::size_t attributeIndex_start = std::tuple_size_v<decltype(Parent::get_attributes())>;
	static constexpr std::size_t attributeIndex_end = attributeIndex_start + std::tuple_size_v<decltype(attributes_part)>;
	template<typename AttributeValueTuple_Type>
	SQLite_Helper_RequestList_Language(const AttributeValueTuple_Type& attributeValues)
		:Parent(attributeValues)
	{
		get_attributeValues_part() = get_tuplePart<attributeIndex_start, attributeIndex_end>(attributeValues);
	}
};

}

#endif /* INCLUDE_DG_LANGUAGES_READER_H_ */
