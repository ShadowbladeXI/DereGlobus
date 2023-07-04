/*
 * settlement.h
 *
 *  Created on: 26 Mar 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_DG_SETTLEMENT_H_
#define DEREGLOBUS_DG_SETTLEMENT_H_

#include "location.h"

namespace dg {

class Settlement : public Location{
public:
	typedef Location DereGlobusParentType;
	typedef unsigned int Population_Type;

	//Class to store population data
	class Population {
	private:
		Population_Type population;
		std::optional<int>yearBF;//TODO: homogenize time types

	public:

		Population(Population_Type population, std::optional<int> yearBF = std::nullopt)
			:population(population), yearBF(yearBF)
		{}

		Population(Population_Type population, int yearBF)
			:Population(population, std::make_optional(yearBF))
		{}

		const Population_Type& get_population() const{
			return population;
		}
		const auto& get_yearBF() const{
			return yearBF;
		}
	};

private:
	//std::optional<unsigned int> population;//No value means that population is unknown/not yet in the database
	std::vector<Population> population;//Empty vector means that population is unknown/not yet in the database
	static constexpr std::string_view sqlStatement_population_string = "SELECT population, time_yearBF FROM population WHERE id = $id  ORDER BY time_yearBF DESC";

public:

	//Class for the information for the SQL request to generate a new object
	class SQL_RequestList : public Location::SQL_RequestList{
	private:
		typedef Location::SQL_RequestList Parent;//Define parent for easier copying of boilerplate

	public:
		//Information in which table and under which name the attributes for the individual class members can be found
		//constexpr static auto population_attribute = SQLite3_Helper_Attribute<unsigned int>("settlement", "population");

		//constexpr static auto attributes_part = std::make_tuple(population_attribute);//Has to be the same number and the same order as in the "get_attributeValues_part" function
		constexpr static auto attributes_part = std::make_tuple();//Has to be the same number and the same order as in the "get_attributeValues_part" function


	public:
		//std::optional<unsigned int> population;

	protected:
		auto get_attributeValues_part(){
			//return std::tie(population);//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
			return std::tie();//Has to be the same number and the same order as in the static "attributes_part" variable //TODO: Is there a way to ensure this better?
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
	Settlement(Location location, std::vector<Population>&& population)
		:Location(location)
		,population(population)
	{}

	virtual ~Settlement(){}

	//Uses data from an SQL Request to create an object.
	//Returns an empty optional if construction failed
	static std::optional<Settlement> make_fromSQLRequest(sqlite3* database, const SQL_RequestList& requestList){
		auto location_opt = Location::make_fromSQLRequest(requestList);
		if(location_opt.has_value() && requestList.id.has_value()){

			//TODO: make Population class responsible for loading population from sql database
			std::vector<Population> population = std::vector<Population>();

			sqlite3_stmt* sqlStatement_population;
			int preparationReturn = sqlite3_prepare_v2(database, sqlStatement_population_string.data(), sqlStatement_population_string.length(), &sqlStatement_population, 0);
			if(preparationReturn != SQLITE_OK){
				//TODO: Add type of error
				throw "Error " + std::to_string(preparationReturn) + " while preparing statement:" + sqlStatement_population_string.data();
			}

			int bindReturn = sqlite3_bind_text(sqlStatement_population, sqlite3_bind_parameter_index(sqlStatement_population, "$id"), requestList.id->c_str(), -1, SQLITE_STATIC);
			if(bindReturn != SQLITE_OK){
				//TODO: Add type of error
				throw "Error " + std::to_string(preparationReturn) + " while binding " + "$id";
			}

			for(int stepReturn; (stepReturn = sqlite3_step(sqlStatement_population)) != SQLITE_DONE; ){
				if(stepReturn == SQLITE_ROW){
					auto population_type = sqlite3_column_type(sqlStatement_population, 0);
					if(population_type != SQLITE_NULL){
						Population_Type population_singleYear = sqlite3_column_int(sqlStatement_population, 0);//TODO: check negative

						auto yearBF_type = sqlite3_column_type(sqlStatement_population, 1);
						if(yearBF_type != SQLITE_NULL){
							int yearBF = sqlite3_column_int(sqlStatement_population, 1);//TODO: time type

							population.emplace_back(population_singleYear, yearBF);
						}else{
							population.emplace_back(population_singleYear);
						}
					}

				}else{
					throw "Error " + std::to_string(stepReturn) + " while reading data";
				}
			}

			sqlite3_finalize(sqlStatement_population);

			return Settlement(*location_opt, std::move(population));
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

	const std::vector<Population>& get_population() const{
		return population;
	}
	//Get most recent population
	std::optional<Population_Type> get_population_recent() const{
		if(population.size() > 0){
			return std::make_optional(population.front().get_population());//TODO: ensure that first one is most recent
		}else{
			return std::nullopt;
		}

	}
};

}

#endif /* DEREGLOBUS_DG_SETTLEMENT_H_ */
