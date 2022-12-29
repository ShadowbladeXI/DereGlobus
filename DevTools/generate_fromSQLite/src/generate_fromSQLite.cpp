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

#include "../includes/rapidxml-1.13/rapidxml.hpp"
#include "../includes/rapidxml-1.13/rapidxml_print.hpp"

#include "sqlite3.h"

#include "../include/sqlite3_tableRead_helper.h"
#include "../include/settlement.h"

using namespace rapidxml;

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
	 * Read settlement information
	 */
	//Create the information to read the table
	auto tableRead_settlements = SQLite3_Helper_TableRead<Settlement::SQL_RequestList>({"location", "settlement"}, {"id", "id"});
	//Read the table to a vector of settlements
	auto settlements = tableRead_settlements.execute_toObjectVector<Settlement>(mainDatabase);

	//Close main database connection
	sqlite3_close(mainDatabase);

	/*
	 * Create Settlements file as a test
	 */

	rapidxml::xml_document<> doc;

	//Add declaration node with standard attributes
	auto* declarationNode = doc.allocate_node(rapidxml::node_declaration);
	auto* declarationNode_version = doc.allocate_attribute("version", "1.0");
	declarationNode->append_attribute(declarationNode_version);
	auto* declarationNode_encoding = doc.allocate_attribute("encoding", "UTF-8");
	declarationNode->append_attribute(declarationNode_encoding);
	auto* declarationNode_standaloneFlag = doc.allocate_attribute("standalone", "yes");
	declarationNode->append_attribute(declarationNode_standaloneFlag);
	doc.append_node(declarationNode);

	//Add kml node
	auto* kmlNode = doc.allocate_node(rapidxml::node_element, "kml");
	auto* kmlNode_xmlns = doc.allocate_attribute("xmlns", "http://www.opengis.net/kml/2.2");
	kmlNode->append_attribute(kmlNode_xmlns);
	auto* kmlNode_xmlnsAtom = doc.allocate_attribute("xmlns:atom", "http://www.w3.org/2005/Atom");
	kmlNode->append_attribute(kmlNode_xmlnsAtom);
	auto* kmlNode_xmlnsGx = doc.allocate_attribute("xmlns:gx", "http://www.google.com/kml/ext/2.2");
	kmlNode->append_attribute(kmlNode_xmlnsGx);
	auto* kmlNode_xmlnsKml = doc.allocate_attribute("xmlns:kml", "http://www.opengis.net/kml/2.2");
	kmlNode->append_attribute(kmlNode_xmlnsKml);
	doc.append_node(kmlNode);

	//Add the main document
	auto* documentNode = doc.allocate_node(rapidxml::node_element, "Document");
	auto* documentNode_name = doc.allocate_node(rapidxml::node_element, "name", "Settlements.kml");
	documentNode->append_node(documentNode_name);
	kmlNode->append_node(documentNode);

	//Add folder for all settlements
	auto* folderNode = doc.allocate_node(rapidxml::node_element, "Folder");
	auto* folderNode_name = doc.allocate_node(rapidxml::node_element, "name", "???");
	folderNode->append_node(folderNode_name);
	documentNode->append_node(folderNode);

	//Add settlements
	for(const auto& settlement : settlements){
		auto* settlement_node = settlement.build_kmlNode(doc);
		folderNode->append_node(settlement_node);
	}

	//For Debugging: display document content in console //TODO: Remove
	std::cout << doc;

	//Write to file
	std::filesystem::create_directory("../../DereGlobus_SQLite/AutoGeneratedContent");
	std::filesystem::create_directory("../../DereGlobus_SQLite/AutoGeneratedContent/Settlements");
	auto file_stream_out = std::ofstream("../../DereGlobus_SQLite/AutoGeneratedContent/Settlements/Settlements.kml");
	file_stream_out << doc;
	file_stream_out.close();

	return 0;
}