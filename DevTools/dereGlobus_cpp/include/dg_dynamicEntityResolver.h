/*
 * dg_dynamicEntityResolver.h
 *
 *  Created on: 9 Apr 2023
 *      Author: Alex
 */

#ifndef INCLUDE_DG_DYNAMICENTITYRESOLVER_H_
#define INCLUDE_DG_DYNAMICENTITYRESOLVER_H_

#include "dg_entities/dereGlobus_entities_fwd.h"

namespace dg{

class DereGlobus_DynamicEntityResolver{
public:

	DereGlobus_DynamicEntityResolver(){}

	virtual ~DereGlobus_DynamicEntityResolver(){}

	void do_typeDeduction(const Entity& entity);

	virtual void do_fixedType(const Entity& entity) = 0;

	virtual void do_fixedType(const Location& location);

	virtual void do_fixedType(const Settlement& settlement);
};

}



#endif /* INCLUDE_DG_DYNAMICENTITYRESOLVER_H_ */
