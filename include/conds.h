#pragma once

#include <vector>
#include <iosfwd>                // for ostream
#include <string>                // for string
#include "../include/cre_obj.h"  // for CRE_Obj
#include "../include/ref.h"      // for ref
#include "../include/literal.h"  // for Literal
#include "../include/t_ids.h"    // for T_ID_CONDITIONS

namespace cre {
struct AllocBuffer;

const uint8_t CONDS_KIND_AND = 1;
const uint8_t CONDS_KIND_OR = 2;

struct Conds : public CRE_Obj {
	static constexpr uint16_t T_ID = T_ID_CONDITIONS;

	// -- Members --
	std::vector<ref<Literal>> literals;
	uint8_t kind;  // CONDS_KIND_AND or CONDS_KIND_OR

	// -- Methods --
	Conds(uint8_t kind, const std::vector<ref<Literal>>& _literals);

	std::string to_string();
};

ref<Conds> new_conds(uint8_t kind, const std::vector<ref<Literal>>& literals, AllocBuffer* buffer = nullptr);

std::ostream& operator<<(std::ostream& out, Conds* conds);
std::ostream& operator<<(std::ostream& out, ref<Conds> conds);

} // NAMESPACE_END(cre)

