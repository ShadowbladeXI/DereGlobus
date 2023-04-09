/*
 * dg_dynamicEntityResolver.cpp
 *
 *  Created on: 9 Apr 2023
 *      Author: Alex
 */

#include "../include/dg_dynamicEntityResolver.h"

#include "../include/dg_entities/dereGlobus_entities.h"

namespace dg{

void DereGlobus_DynamicEntityResolver::do_typeDeduction(const Entity& entity){
	entity.execute_resolved(*this);
}

void DereGlobus_DynamicEntityResolver::do_fixedType(const Location& location){
	do_fixedType(static_cast<std::remove_reference_t<decltype(location)>::DereGlobusParentType>(location));
}

void DereGlobus_DynamicEntityResolver::do_fixedType(const Settlement& settlement){
	do_fixedType(static_cast<std::remove_reference_t<decltype(settlement)>::DereGlobusParentType>(settlement));
}

}


