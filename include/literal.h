#pragma once

#include "../include/types.h"
#include "../include/cre_obj.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/ref.h"
#include "../include/var.h"

namespace cre {

const uint8_t LIT_KIND_EQ = 1;
const uint8_t LIT_KIND_FACT = 2 ;
const uint8_t LIT_KIND_FUNC = 3 ;


struct Literal {//: public CRE_Obj{
	static constexpr uint16_t T_ID = T_ID_LITERAL;

	Item obj; // A fact, func, or value
	std::vector<ref<Var>> vars;
	std::vector<int32_t> var_inds;
	float weight; 
	bool negated;	
	uint8_t kind;
	uint8_t pad[2];


	// Literal()
};

} // NAMESPACE_END(cre)
