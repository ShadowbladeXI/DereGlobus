/*
 * test_buildExample.cpp
 *
 *  Created on: 9 Apr 2023
 *      Author: Alex
 */

#include <iostream>
#include <fstream>
#include <tuple>
#include <filesystem>
#include <unordered_map>
#include <memory>

#include "sqlite3.h"

#include "../include/dg_entities/dereGlobus_entities.h"
#include "../include/dg_languages_reader.h"
#include "../include/texts_map.h"

using namespace dg;

int main(){
	//Open main database
	sqlite3* mainDatabase;
	int openingReturn = sqlite3_open("../../mainDatabase.sqlite3", &mainDatabase);//TODO: make dedicated tst library
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
	auto tableRead_languages = SQLite3_Helper_TableRead<SQLite_Helper_RequestList_Language>("language");

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
	auto texts_map = Texts_Map(languages);
	//auto language_attributes = std::vector<SQLite3_Helper_Attribute<std::string>>();
	auto languageAndId_attributes = std::vector<SQLite3_Helper_Attribute<std::string>>();
	languageAndId_attributes.reserve(languages.size()+1);
	languageAndId_attributes.push_back(SQLite3_Helper_Attribute<std::string>("text", "text_id"));
	for(const std::string& language_id : languages){
		languageAndId_attributes.push_back(SQLite3_Helper_Attribute<std::string>("text", language_id));
	}
	auto tableRead_texts = SQLite3_Helper_TableRead_Dynamic<std::string>(languageAndId_attributes, "text");
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
	auto tableRead_settlements = SQLite3_Helper_TableRead<Settlement::SQL_RequestList>({"entity", "location", "settlement"}, {"id", "id", "id"});
	//Read the table to a vector of settlements
	auto settlements = tableRead_settlements.execute_toObjectVector<Settlement>(mainDatabase);

	//Close main database connection
	sqlite3_close(mainDatabase);

	return 0;
}


