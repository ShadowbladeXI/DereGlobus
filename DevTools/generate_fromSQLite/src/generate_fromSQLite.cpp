//============================================================================
// Name        : generate_fromSQLite.cpp
// Author      : ShadowbladeXI
// Version     :
// Copyright   : //TODO: make in separate file
// Description : Program to generate (part of) the files for DereGlobus from an SQLite database
//============================================================================

#include <iostream>
#include <fstream>
#include <tuple>
#include <filesystem>
#include <unordered_map>
#include <memory>

#include "sqlite3.h"

//TODO: Remove
#include "../lib/rapidxml-1.13/rapidxml.hpp"
#include "../lib/rapidxml-1.13/rapidxml_print.hpp"

#include "kml.h"
#include "dereGlobus.h"

#include "../include/dereGlobus_kml/dgkml_entities.h"
#include "../include/dereGlobus_kml/dgkml_factory.h"

//Class for reading all languages //TODO: move to other location
class SQLite_Helper_RequestList_Language : dg::SQLite_Helper_RequestList{
private:
	typedef dg::SQLite_Helper_RequestList Parent;//Define parent for easier copying of boilerplate

private:
	constexpr static auto id_attribute = dg::SQLite3_Helper_Attribute<std::string>("language", "language_id");
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



int main() {

	//Open main database
	sqlite3* mainDatabase;
	int openingReturn = sqlite3_open("../../mainDatabase.sqlite3", &mainDatabase);
	if(openingReturn){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(mainDatabase));
		sqlite3_close(mainDatabase);
		return(1);
	}

	/*
	 * Read texts (in different languages)
	 */
	// Get all languages
	//Create the information to read the table
	auto tableRead_languages = dg::SQLite3_Helper_TableRead<dg::SQLite_Helper_RequestList_Language>("language");

	//Read the table
	auto languages_requests = tableRead_languages.execute_toRequestVector(mainDatabase);

	auto languages = std::vector<std::string>();
	languages.reserve(languages_requests.size());
	for(auto language_request : languages_requests){
		const auto& language_id_opt = language_request.id;
		if(language_id_opt.has_value()){
			languages.push_back(*(language_request.id));
		}else{
			throw "Language ID cannot be empty!";
		}
	}

	//get the texts
	auto texts_map = dg::Texts_Map(languages);
	//auto language_attributes = std::vector<SQLite3_Helper_Attribute<std::string>>();
	auto languageAndId_attributes = std::vector<dg::SQLite3_Helper_Attribute<std::string>>();
	languageAndId_attributes.reserve(languages.size()+1);
	languageAndId_attributes.push_back(dg::SQLite3_Helper_Attribute<std::string>("text", "text_id"));
	for(const std::string& language_id : languages){
		languageAndId_attributes.push_back(dg::SQLite3_Helper_Attribute<std::string>("text", language_id));
	}
	auto tableRead_texts = dg::SQLite3_Helper_TableRead_Dynamic<std::string>(languageAndId_attributes, "text");
	auto function_addToTexts = [&texts_map](const std::vector<std::optional<std::string>>& languageAndId_values){
		std::optional<std::string> text_id_optional = languageAndId_values.front();
		if(text_id_optional.has_value()){
			const std::string& text_id = *text_id_optional;

			auto translations = std::vector<std::optional<std::string>>(languageAndId_values.begin()+1, languageAndId_values.end());

			auto [insert_it, success] = texts_map.insert(std::make_pair(text_id, translations));
			if(!success){
				throw "Text with ID " + insert_it->first + " is duplicate!";
			}
		}else{
			throw "Language ID cannot be empty!";
		}

	};
	tableRead_texts.execute(mainDatabase, function_addToTexts);

	/*
	 * Read settlement information
	 */
	//Create the information to read the table
	auto tableRead_settlements = dg::SQLite3_Helper_TableRead<dg::Settlement::SQL_RequestList>({"entity", "location", "settlement"}, {"id", "id", "id"});
	//Read the table to a vector of settlements
	auto settlements = tableRead_settlements.execute_toObjectVector<dg::Settlement>(mainDatabase);

	//Close main database connection
	sqlite3_close(mainDatabase);

	/*
	 * Create dereGlobus kml objects
	 */

	auto settlements_doc = std::make_unique<kml::Document>();
	settlements_doc->set_name("Settlements.kml");
	kml::Folder& settlements_folder = settlements_doc->add_feature(std::make_unique<kml::Folder>());
	settlements_folder.set_name("???");

	auto dgkml_factory = DG_KML_Factory();
	for(const auto& settlement : settlements){
		auto settlement_dgkml = dgkml_factory.make_typeDeduction(texts_map, settlement);
		settlements_folder.add_feature(std::move(settlement_dgkml));
	}

	/*
	 * Create Settlements file as a test
	 */

	auto kml_file = kml::File();
	kml_file.add_feature(std::move(settlements_doc));

	//Write to file
	auto xml_doc = rapidxml::xml_document<kml::Ch_T>();
	kml_file.create_xmlDocument(xml_doc);
	std::filesystem::create_directory("../../DereGlobus_SQLite/AutoGeneratedContent");
	std::filesystem::create_directory("../../DereGlobus_SQLite/AutoGeneratedContent/Settlements");
	auto file_stream_out = std::ofstream("../../DereGlobus_SQLite/AutoGeneratedContent/Settlements/Settlements.kml");
	file_stream_out << xml_doc;
	file_stream_out.close();
	xml_doc.clear();


	return 0;
}

