/*
 * sqlite3_tableRead_helper.h
 *
 * List of convenience functionality to read data from tables of an SQL server
 *
 *  Created on: Dec 25, 2022
 *      Author: ShadowbladeXI
 */

#ifndef SQLITE3_TABLEREAD_HELPER_H_
#define SQLITE3_TABLEREAD_HELPER_H_

#include <array>
#include <vector>
#include <tuple>
#include <string_view>
#include <optional>
#include <algorithm>
#include <assert.h>

#include "sqlite3.h"
#include "../external/math/tuple_math.h"

namespace dg{

/*
 * Advance Declaration
 */

//Class to store a single attribute
//"Attribute_Type" is the type of value of this attribute
template<typename Attribute_Type>
class SQLite3_Helper_Attribute{
public:
	typedef Attribute_Type A_Type;

	const std::string_view table_name;
	const std::string_view attribute_name;

public:
	//provide the table in which the attribute is found under "attribute_name"
	constexpr SQLite3_Helper_Attribute(std::string_view table_name, std::string_view attribute_name)
		:table_name(table_name)
		,attribute_name(attribute_name)
	{}
};

//Class to store values for reading values from a table
//Subclasses have to be defined to store the read values ("SQLite_Helper_RequestList_Dummy" is a template that can be copied for that)
class SQLite_Helper_RequestList{
public:
	/*
	 * Boilerplate interface for creating new instances, these methods have to be defined properly for children
	 */
	//return tuple of "SQLite3_Helper_Attribute" references in the same order as the corresponding results should be provided to the constructor
	static auto get_attributes(){
		return std::make_tuple();
	}

	//Create an object based on a tuple of std::optional values which are the results of requesting the values of the attributes provided with the "get_attributes" method (also in the same order)
	template<typename AttributeValueTuple_Type>
	SQLite_Helper_RequestList(const AttributeValueTuple_Type&){}
};
//Dummy class which can be copied and used as a template to create SQLite_Helper_RequestList children
//This template can easily be extended by further child classes (just the "Parent" typedef has to be changed as well)
class SQLite_Helper_RequestList_Dummy : SQLite_Helper_RequestList{
private:
	typedef SQLite_Helper_RequestList Parent;//Define parent for easier copying of boilerplate

private:
	//Information in which table and under which name the attributes for the individual class members can be found
	//Replace with own requests
	constexpr static auto dummy1_attribute = SQLite3_Helper_Attribute<std::string>("dummyTable", "dummy1");//value of type std::string requested
	constexpr static auto dummy2_attribute = SQLite3_Helper_Attribute<double>("dummyTable", "dummy2");//value of type double requested

	//list own requests
	constexpr static auto attributes_part = std::make_tuple(dummy1_attribute, dummy2_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function

public:
	//storage for the requested attribute values
	//Replace with own variables
	std::optional<std::string> dummy1;
	std::optional<double> dummy2;

protected:
	//list own variables
	auto get_attributeValues_part(){
		return std::tie(dummy1, dummy2);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
	}

public:
	/*
	 * Boilerplate interface for creating new instances, can be copied without modification except constructor name
	 */
	static auto get_attributes(){
		return std::tuple_cat(Parent::get_attributes(), attributes_part);
	}

	static constexpr std::size_t attributeIndex_start = std::tuple_size_v<decltype(Parent::get_attributes())>;
	static constexpr std::size_t attributeIndex_end = attributeIndex_start + std::tuple_size_v<decltype(attributes_part)>;
	template<typename AttributeValueTuple_Type>
	SQLite_Helper_RequestList_Dummy(const AttributeValueTuple_Type& attributeValues)
		:Parent(attributeValues)
	{
		get_attributeValues_part() = get_tuplePart<attributeIndex_start, attributeIndex_end>(attributeValues);
	}
};


//Helper class for reading the content of an entire table or multiple tables joined on a single attribute for all tables
//"SQL_RequestList_Type" has to be a subclass of "SQLite_Helper_RequestList"
template<typename SQL_RequestList_Type>
class SQLite3_Helper_TableRead{
private:
	const std::vector<std::string> table_names;//Names of all tables from which content should be read
	const std::vector<std::string> idAttribute_names;//Names of the common id attribute for all tables

public:
	//Create a class for reading from multiple tables joined on the attributes in "idAttribute_names"
	SQLite3_Helper_TableRead(std::vector<std::string> table_names, std::vector<std::string> idAttribute_names)
		:table_names(table_names)
		,idAttribute_names(idAttribute_names)
	{
		assert(table_names.size() == idAttribute_names.size() || (table_names.size() == 1 && idAttribute_names.empty()));
		//check that all the required tables of "SQL_RequestList_Type" are in "table_names"
		if(!check_rerquiredTables(SQL_RequestList_Type::get_attributes())){
			//TODO: make dedicated exception
			std::cout << "Not all required tables are named!" << std::endl;
			throw "Not all required tables are named!";
		}
	}

	//Create a class for reading from a single table
	SQLite3_Helper_TableRead(const std::string& table_name)
		:SQLite3_Helper_TableRead(std::vector<std::string>(1, table_name), std::vector<std::string>())
	{}

private:
	template<typename AttributesTuple_Type, std::size_t fromInd = 0>
	bool check_rerquiredTables(const AttributesTuple_Type& attributes_tuple) const{
		if constexpr(fromInd >= std::tuple_size_v<AttributesTuple_Type>){
			return true;
		}else{
			const std::string_view& table_name = std::get<fromInd>(attributes_tuple).table_name;
			if(std::find(table_names.begin(), table_names.end(), table_name) == table_names.end()){
				return false;
			}else{
				return check_rerquiredTables<AttributesTuple_Type, fromInd+1>(attributes_tuple);
			}
		}
	}

	std::string create_SQLiteStatement_string() const{
		std::string statement_string = "";

		//list attributes
		statement_string += "SELECT\n\r";
		create_SQLiteStatement_string_helper_selectRecursion(statement_string, SQL_RequestList_Type::get_attributes());

		//list tables
		statement_string += "FROM\n\r";
		for(std::size_t tableIndex = 0; tableIndex < table_names.size(); ++tableIndex){
			const std::string& table_name = table_names.at(tableIndex);
			statement_string += ("\t" + table_name + " AS " + table_name);
			if(tableIndex == table_names.size()-1){
				statement_string += "\n\r";
			}else{
				statement_string += ",\n\r";
			}

		}

		//list constraints
		statement_string += "WHERE\n\r";
		statement_string += "\t0=0\n\r";
		for(std::size_t tableIndex = 1; tableIndex < table_names.size(); ++tableIndex){
			statement_string += ("\tAND " + table_names.front()+"."+idAttribute_names.front() + " = " + table_names.at(tableIndex)+"."+idAttribute_names.at(tableIndex) + "\n\r");
		}

		//For Debugging
		//std::cout << "Assembled Statement:" << std::endl;
		//std::cout << statement_string << std::endl;

		return statement_string;
	}

	//Helper method for the recursion required to extract all the requested attributes to a string
	template<typename Tuple_Type, std::size_t fromInd = 0>
	void create_SQLiteStatement_string_helper_selectRecursion(std::string& toAppendTo, const Tuple_Type& attributes_tuple) const{
		constexpr std::size_t tupleSize = std::tuple_size_v<Tuple_Type>;
		if constexpr(fromInd >= tupleSize){
			return;
		}else{
			auto attribute = std::get<fromInd>(attributes_tuple);
			((((toAppendTo += "\t") += attribute.table_name) += ".") += attribute.attribute_name);
			if constexpr(fromInd == tupleSize - 1){
				toAppendTo += "\n\r";//No "," after the last attribute
			}else{
				toAppendTo += ",\n\r";
			}

			create_SQLiteStatement_string_helper_selectRecursion<Tuple_Type, fromInd+1>(toAppendTo, attributes_tuple);
			return;
		}
	}

	//Create and prepare SQLite3 statement
	sqlite3_stmt* create_SQLiteStatement(sqlite3* database) const{
		const std::string& queryStatement_string = create_SQLiteStatement_string();
		sqlite3_stmt* queryStatement_prepared;
		auto preparationReturn = sqlite3_prepare_v2(database, queryStatement_string.c_str(), -1, &queryStatement_prepared, 0);

		if(preparationReturn == SQLITE_OK){
			return queryStatement_prepared;
		}else{
			//TODO: Add type of error
			throw "Error " + std::to_string(preparationReturn) + " while preparing statement:" + queryStatement_string;
		}


	}

	//Helper method for the recursion required to read a row of values into a tuple
	//Result will be written into "attributeValues_tuple"
	template<typename Tuple_Type, std::size_t fromColumn = 0>
	void execute_helper_singleRowRecursion(Tuple_Type& attributeValues_tuple, sqlite3_stmt* sqlStatement) const{
		constexpr std::size_t tupleSize = std::tuple_size_v<Tuple_Type>;
		if constexpr(fromColumn >= tupleSize){
			return;
		}else{
			typedef std::tuple_element_t<fromColumn, Tuple_Type> ValueOptional_Type;
			typedef typename ValueOptional_Type::value_type Value_Type;

			ValueOptional_Type attributeValue;
			auto attributeType_sqlite3 = sqlite3_column_type(sqlStatement, fromColumn);

			//Check if original database entry was NULL
			if(attributeType_sqlite3 == SQLITE_NULL){
				//std::cout << "NULL, ";
				attributeValue = std::nullopt;
			}else{
				sqlite3_value* attributeValue_sqlite3 = sqlite3_column_value(sqlStatement, fromColumn);
				sqlite3_value* attributeValue_sqlite3_protected = sqlite3_value_dup(attributeValue_sqlite3);

				if constexpr(std::is_same_v<std::string, Value_Type>){
					//std::cout << "string, ";
					//assert(attributeType_sqlite3 == SQLITE_TEXT);
					attributeValue = reinterpret_cast<const char*>(sqlite3_value_text(attributeValue_sqlite3_protected));
				}else if constexpr(std::is_same_v<double, Value_Type>){
					//std::cout << "double, ";
					//assert(attributeType_sqlite3 == SQLITE_FLOAT);
					attributeValue = sqlite3_value_double(attributeValue_sqlite3_protected);
				}else if constexpr(std::is_same_v<unsigned int, Value_Type>){
					//std::cout << "unsigned int, ";
					//assert(attributeType_sqlite3 == SQLITE_INTEGER);
					attributeValue = sqlite3_value_int(attributeValue_sqlite3_protected);
				}else if constexpr(std::is_same_v<int, Value_Type>){
					//std::cout << "int, ";
					//assert(attributeType_sqlite3 == SQLITE_INTEGER);
					attributeValue = sqlite3_value_int(attributeValue_sqlite3_protected);
				}else{
					//std::cout << "error for type " << typeid(Value_Type()).name() << ", ";//For debugging

					//If this is reached, an unsupported type is requested to be read
					static_assert(!std::is_same_v<Value_Type, Value_Type>);//Will always be false
				}
			}

			std::get<fromColumn>(attributeValues_tuple) = attributeValue;//fill into tuple
			execute_helper_singleRowRecursion<Tuple_Type, fromColumn+1>(attributeValues_tuple, sqlStatement);//recursion call
			return;
		}
	}

	//Not meant to be called, just for determining the tuple type via decltype //TODO: make a more elegant solution?
	template<typename AttributesTuple_Type, std::size_t fromRow = 0>
	auto execute_helper_tupleTypeRecursion() const{
		if constexpr(fromRow >= std::tuple_size_v<AttributesTuple_Type>) {
			return std::make_tuple();
		}else{
			auto dummy = execute_helper_tupleTypeRecursion<AttributesTuple_Type, fromRow+1>();
			typedef typename std::tuple_element_t<fromRow, AttributesTuple_Type>::A_Type Value_Type;
			typedef std::optional<Value_Type> ValueOptional_Type;
			return std::tuple_cat(std::make_tuple(ValueOptional_Type()), dummy);
		}
	}

public:
	//Read the table and execute the given function for each row (taking the corresponding SQL_RequestList_Type object as a parameter)
	template<typename Function_Type>
	void execute(sqlite3* database, Function_Type function) const{

		sqlite3_stmt* queryStatement_prepared;
		try{
			queryStatement_prepared = create_SQLiteStatement(database);
		}catch(const std::string& exception_message){
			std::cout << exception_message << std::endl;
			return;
		}

		for(int stepReturn; (stepReturn = sqlite3_step(queryStatement_prepared)) != SQLITE_DONE; ){
			if(stepReturn == SQLITE_ROW){
				//std::cout << "Reading Row" << std::endl;

				typedef decltype(execute_helper_tupleTypeRecursion<decltype(SQL_RequestList_Type::get_attributes())>()) Tuple_Type;
				Tuple_Type resultTuple;
				execute_helper_singleRowRecursion(resultTuple, queryStatement_prepared);

				//Create request object
				auto request = SQL_RequestList_Type(resultTuple);

				//User function call
				function(request);
			}else{
				throw "Error " + std::to_string(stepReturn) + " while reading data";
			}
		}

		sqlite3_finalize(queryStatement_prepared);
		return;
	}

	//Convenience method for reading the table and storing the results in a vector
	std::vector<SQL_RequestList_Type> execute_toRequestVector(sqlite3* database){
		std::vector<SQL_RequestList_Type> requests;
		auto function = [&requests](const SQL_RequestList_Type& request){requests.push_back(request);};
		execute(database, function);
		return requests;
	}

	//Convenience method for reading the table, creating an object of type "GeneratedObject_Type" for each row and storing the created objects in a vector
	//"GeneratedObject_Type" has to have a static "make_fromSQLRequest" method that takes a "SQL_RequestList_Type" and creates a "std::optional<SQL_RequestList_Type>" (no contained value means that object creation failed for the provided "SQL_RequestList_Type" object)
	template<typename GeneratedObject_Type>
	std::vector<GeneratedObject_Type> execute_toObjectVector(sqlite3* database){
		std::vector<GeneratedObject_Type> objects;
		auto function = [&objects, &database](const SQL_RequestList_Type& request){
			auto object_optional = GeneratedObject_Type::make_fromSQLRequest(database, request);
			if(object_optional.has_value()){
				const auto& object = *object_optional;
				objects.push_back(object);
			}else{
				//TODO: make exception?
				std::cout << "Entity creation failed" << std::endl;
			}
		};
		execute(database, function);
		return objects;

	}


};

//Helper class for reading the content of an entire table or multiple tables joined on a single attribute for all tables //TODO: Merge with static version
//This version is for dynamic requests
template<typename Value_Type>
class SQLite3_Helper_TableRead_Dynamic{
private:
	typedef SQLite3_Helper_Attribute<Value_Type> Attr_Type;

	const std::vector<std::string> table_names;//Names of all tables from which content should be read
	const std::vector<std::string> idAttribute_names;//Names of the common id attribute for all tables
	const std::vector<Attr_Type> attributes;//Requested attributes

public:
	//Create a class for reading from multiple tables joined on the attributes in "idAttribute_names"
	SQLite3_Helper_TableRead_Dynamic(std::vector<Attr_Type> attributes, std::vector<std::string> table_names, std::vector<std::string> idAttribute_names)
		:table_names(table_names)
		,idAttribute_names(idAttribute_names)
		,attributes(attributes)
	{
		assert(table_names.size() == idAttribute_names.size() || (table_names.size() == 1 && idAttribute_names.empty()));
		//TODO: Add check that all the required tables of "attributes" are in "table_names"
	}

	//Create a class for reading from a single table
	SQLite3_Helper_TableRead_Dynamic(std::vector<Attr_Type> attributes, const std::string& table_name)
		:SQLite3_Helper_TableRead_Dynamic(attributes, std::vector<std::string>(1, table_name), std::vector<std::string>())
	{}

private:

	std::string create_SQLiteStatement_string() const{
		std::string statement_string = "";

		//list attributes
		statement_string += "SELECT\n\r";
		for(std::size_t attribute_index = 0; attribute_index < attributes.size(); ++attribute_index){
			const auto& attribute = attributes.at(attribute_index);
			((((statement_string += "\t") += attribute.table_name) += ".") += attribute.attribute_name);
			if(attribute_index == attributes.size()-1){
				statement_string += "\n\r";//no "," for last attribute
			}else{
				statement_string += ",\n\r";
			}
		}

		//list tables
		statement_string += "FROM\n\r";
		for(std::size_t tableIndex = 0; tableIndex < table_names.size(); ++tableIndex){
			const std::string& table_name = table_names.at(tableIndex);
			statement_string += ("\t" + table_name + " AS " + table_name);
			if(tableIndex == table_names.size()-1){
				statement_string += "\n\r";
			}else{
				statement_string += ",\n\r";
			}

		}

		//list constraints
		statement_string += "WHERE\n\r";
		statement_string += "\t0=0\n\r";
		for(std::size_t tableIndex = 1; tableIndex < table_names.size(); ++tableIndex){
			statement_string += ("\tAND " + table_names.front()+"."+idAttribute_names.front() + " = " + table_names.at(tableIndex)+"."+idAttribute_names.at(tableIndex) + "\n\r");
		}

		//For Debugging
		std::cout << "Assembled Statement:" << std::endl;
		std::cout << statement_string << std::endl;

		return statement_string;
	}

	//Create and prepare SQLite3 statement
	sqlite3_stmt* create_SQLiteStatement(sqlite3* database) const{
		const std::string& queryStatement_string = create_SQLiteStatement_string();
		sqlite3_stmt* queryStatement_prepared;
		auto preparationReturn = sqlite3_prepare_v2(database, queryStatement_string.c_str(), -1, &queryStatement_prepared, 0);

		if(preparationReturn == SQLITE_OK){
			return queryStatement_prepared;
		}else{
			//TODO: Add type of error
			throw "Error " + std::to_string(preparationReturn) + " while preparing statement:\n\r" + queryStatement_string;
		}
	}

public:
	//Read the table and execute the given function for each row (taking a vector of std::optional<Value_Type> as a parameter)
	template<typename Function_Type>
	void execute(sqlite3* database, Function_Type function) const{

		sqlite3_stmt* queryStatement_prepared;
		try{
			queryStatement_prepared = create_SQLiteStatement(database);
		}catch(const std::string& exception_message){
			std::cout << exception_message << std::endl;
			return;
		}
		int nAttributes = sqlite3_column_count(queryStatement_prepared);

		for(int stepReturn; (stepReturn = sqlite3_step(queryStatement_prepared)) != SQLITE_DONE; ){
			if(stepReturn == SQLITE_ROW){
				//std::cout << "Reading Row" << std::endl;

				std::vector<std::optional<Value_Type>> results_vector;
				results_vector.reserve(nAttributes);
				for(int attribute_index = 0; attribute_index < nAttributes; ++attribute_index){

					std::optional<Value_Type> attributeValue;
					auto attributeType_sqlite3 = sqlite3_column_type(queryStatement_prepared, attribute_index);

					//Check if original database entry was NULL
					if(attributeType_sqlite3 == SQLITE_NULL){
						//std::cout << "NULL, ";
						attributeValue = std::nullopt;
					}else{
						sqlite3_value* attributeValue_sqlite3 = sqlite3_column_value(queryStatement_prepared, attribute_index);
						sqlite3_value* attributeValue_sqlite3_protected = sqlite3_value_dup(attributeValue_sqlite3);

						if constexpr(std::is_same_v<std::string, Value_Type>){
							//std::cout << "string, ";
							attributeValue = reinterpret_cast<const char*>(sqlite3_value_text(attributeValue_sqlite3_protected));
						}else if constexpr(std::is_same_v<double, Value_Type>){
							//std::cout << "double, ";
							attributeValue = sqlite3_value_double(attributeValue_sqlite3_protected);
						}else if constexpr(std::is_same_v<unsigned int, Value_Type>){
							//std::cout << "unsigned int, ";
							attributeValue = sqlite3_value_int(attributeValue_sqlite3_protected);
						}else{
							//std::cout << "error for type " << typeid(Value_Type()).name() << ", ";//For debugging

							//If this is reached, an unsupported type is requested to be read
							static_assert(!std::is_same_v<Value_Type, Value_Type>);//Will always be false
						}
					}
					results_vector.push_back(attributeValue);
				}

				//User function call
				function(results_vector);
			}else{
				throw "Error " + std::to_string(stepReturn) + " while reading data";
			}
		}

		sqlite3_finalize(queryStatement_prepared);
		return;
	}

	//Convenience method for reading the table and storing the results in a vector of vectors
	std::vector<std::vector<std::optional<Value_Type>>> execute_toVectors(sqlite3* database){
		std::vector<std::vector<std::optional<Value_Type>>> values;
		auto function = [&values](const std::vector<std::optional<Value_Type>>& values_singleRow){values.push_back(values_singleRow);};
		execute(database, function);
		return values;
	}

};

}

#endif /* SQLITE3_TABLEREAD_HELPER_H_ */
