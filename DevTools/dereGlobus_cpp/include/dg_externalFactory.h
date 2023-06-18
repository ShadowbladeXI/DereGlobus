/*
 * dg_externalFactory.h
 *
 *  Created on: 9 Apr 2023
 *      Author: Alex
 */

#ifndef DEREGLOBUS_DG_EXTERNALFACTORY_H_
#define DEREGLOBUS_DG_EXTERNALFACTORY_H_

#include <memory>

#include "dg_entities/dereGlobus_entities.h"
#include "dg_dynamicEntityResolver.h"

namespace dg{

//Inherit from this class and overwrite the make_fixedType methods to easily create your own objects for different dereGlobus entities
//Non-overwritten functions default to call the function for the next parent in the dereGlobus inheritance structure
template<typename Output_BaseType>
class DereGlobus_ExternalFactory : private DereGlobus_DynamicEntityResolver{

protected:
	//Intrmdiat storage for object
	std::unique_ptr<Output_BaseType> object = nullptr;

public:
	std::unique_ptr<Output_BaseType> make_typeDeduction(const Entity& entity) {
		do_typeDeduction(entity);
		return std::move(object);
	}


private:
	/*
	 * To Overwrite
	 */
	virtual std::unique_ptr<Output_BaseType> make_fixedType(const Entity& entity) const = 0;

	virtual std::unique_ptr<Output_BaseType> make_fixedType(const Location& location) const{
		return make_fixedType(static_cast<std::remove_reference_t<decltype(location)>::DereGlobusParentType>(location));
	}

	virtual std::unique_ptr<Output_BaseType> make_fixedType(const Settlement& settlement) const{
		return make_fixedType(static_cast<std::remove_reference_t<decltype(settlement)>::DereGlobusParentType>(settlement));
	}

	virtual std::unique_ptr<Output_BaseType> make_fixedType(const Town& town) const{
		return make_fixedType(static_cast<std::remove_reference_t<decltype(town)>::DereGlobusParentType>(town));
	}

private:
	void do_fixedType(const Entity& entity) override{
		object = std::move(make_fixedType(entity));
	}

	void do_fixedType(const Location& location) override{
		object = std::move(make_fixedType(location));
	}

	void do_fixedType(const Settlement& settlement) override{
		object = std::move(make_fixedType(settlement));
	}

	void do_fixedType(const Town& town) override{
		object = std::move(make_fixedType(town));
	}

};

}

#endif /* DEREGLOBUS_DG_EXTERNALFACTORY_H_ */
