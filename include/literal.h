#pragma once
	
#include <vector>
// #include "../include/types.h"
// #include "../include/cre_obj.h"
// #include "../include/hash.h"
// #include "../include/intern.h"
#include "../include/ref.h"
#include "../include/var.h"

namespace cre {

struct AllocBuffer;

const uint8_t LIT_KIND_EQ = 1;
const uint8_t LIT_KIND_VAR = 2 ;
const uint8_t LIT_KIND_FACT = 3 ;
const uint8_t LIT_KIND_FUNC = 4 ;


struct Literal : public CRE_Obj{
	static constexpr uint16_t T_ID = T_ID_LITERAL;

	ref<CRE_Obj> obj; // A fact, func, or value
	std::vector<ref<Var>> vars = {};
	std::vector<int32_t> var_inds = {};
	float weight; 
	bool negated;	
	uint8_t kind;
	uint8_t pad[2];

	Literal(CRE_Obj* obj, bool negated=false);
	std::string to_string(uint8_t verbosity=DEFAULT_VERBOSITY);

	inline bool is_func() const {
		return kind == LIT_KIND_FUNC || kind == LIT_KIND_EQ;
	}

	CRE_Type* eval_type() const;
	uint16_t eval_t_id() const;
};


ref<Literal> new_literal(CRE_Obj* obj, bool negated=false, AllocBuffer* buffer=nullptr);


std::ostream& operator<<(std::ostream& out, Literal* func);
std::ostream& operator<<(std::ostream& out, ref<Literal> func);

} // NAMESPACE_END(cre)
