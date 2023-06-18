/*
 * town.h
 *
 *  Created on: 13 May 2023
 *      Author: Alex
 */

#ifndef INCLUDE_DG_ENTITIES_TOWN_H_
#define INCLUDE_DG_ENTITIES_TOWN_H_

#include "location.h"

namespace dg {

class Town : public Settlement{
public:
	typedef Settlement DereGlobusParentType;

	Town(Settlement settlement)
		:Settlement(settlement)
	{}

	virtual ~Town(){}

	//Uses data from an SQL Request to create an object.
	//Returns an empty optional if construction failed
	static std::optional<Town> make_fromSQLRequest(const SQL_RequestList& requestList){
		auto settlement_opt = Settlement::make_fromSQLRequest(requestList);
		if(settlement_opt.has_value()){
			return Town(*settlement_opt);
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

#endif /* INCLUDE_DG_ENTITIES_TOWN_H_ */
