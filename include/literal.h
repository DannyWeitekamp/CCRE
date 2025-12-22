#pragma once
	
#include <vector>
// #include "../include/types.h"
// #include "../include/cre_obj.h"
// #include "../include/hash.h"
// #include "../include/intern.h"
#include "../include/ref.h"
#include "../include/var.h"
#include "../include/func.h"
#include "../include/var_inds.h" // for VarInds

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
	VarInds var_inds = {};
	float structure_weight = 1.0f; 
	float match_weight = 1.0f; 
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

	bool operator==(const Literal& other) const;
};


ref<Literal> new_literal(CRE_Obj* obj, bool negated=false, AllocBuffer* buffer=nullptr);


std::ostream& operator<<(std::ostream& out, Literal* func);
std::ostream& operator<<(std::ostream& out, ref<Literal> func);

inline auto FuncRef::operator~() const {
	return new_literal((CRE_Obj*) this->get(), true);
}

bool literals_equal(const Literal* lit1, const Literal* lit2, bool semantic=true);
// bool Literal::operator==(const Literal& other) const;

} // NAMESPACE_END(cre)
