/*
 * tuple_math.h
 *
 *  Created on: Dec 28, 2022
 *      Author: ShadowbladeXI
 */

#ifndef MATH_TUPLE_MATH_H_
#define MATH_TUPLE_MATH_H_

#include <tuple>

/*
 * get part of a tuple as separate tuple
 */
//Defined by start and past-the-end index
template<std::size_t index_start, std::size_t index_end, typename Tuple_Type>
auto get_tuplePart(const Tuple_Type& tuple){
	if constexpr(index_start >= index_end){
		return std::tuple<>();
	}else{
		return std::tuple_cat(std::make_tuple(std::get<index_start>(tuple)), get_tuplePart<index_start+1, index_end, Tuple_Type>(tuple));
	}
}



#endif /* MATH_TUPLE_MATH_H_ */
