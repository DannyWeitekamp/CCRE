#include "../include/conds.h"
#include "../include/alloc_buffer.h"
#include "cre_obj.h"           // for CRE_Obj, alloc_cre_obj, CRE_Obj_dtor
#include "types.h"             // for T_ID
#include <sstream>              // for stringstream
#include <stdexcept>           // for runtime_error

namespace cre {

void Conds_dtor(const CRE_Obj* x) {
	Conds* conds = (Conds*) x;
	// Vector of refs will automatically handle reference counting
	CRE_Obj_dtor(x);
}

Conds::Conds(bool is_and) :
	kind(is_and ? CONDS_KIND_AND : CONDS_KIND_OR)
{
	literals = {};
}

Conds::Conds(const std::vector<ref<Literal>>& _literals, bool is_and) :
	literals(_literals),
	kind(is_and ? CONDS_KIND_AND : CONDS_KIND_OR)
{
}

ref<Conds> new_conds(bool is_and, AllocBuffer* buffer) {
	auto [addr, did_malloc] = alloc_cre_obj(sizeof(Conds), &Conds_dtor, T_ID_CONDITIONS, buffer);
	Conds* conds = new (addr) Conds(is_and);
	return conds;
}

ref<Conds> new_conds(const std::vector<ref<Literal>>& literals, bool is_and, AllocBuffer* buffer) {
	auto [addr, did_malloc] = alloc_cre_obj(sizeof(Conds), &Conds_dtor, T_ID_CONDITIONS, buffer);
	Conds* conds = new (addr) Conds(literals, is_and);
	return conds;
}

std::string Conds::to_string() {
	std::stringstream ss;
	
	std::string op = (kind == CONDS_KIND_AND) ? " & " : " | ";
	
	if (literals.empty()) {
		ss << (kind == CONDS_KIND_AND ? "AND()" : "OR()");
	} else {
		for (size_t i = 0; i < literals.size(); ++i) {
			if (i > 0) {
				ss << op;
			}
			ss << literals[i].get()->to_string();
		}
	}
	
	return ss.str();
}

std::ostream& operator<<(std::ostream& out, Conds* conds) {
	return out << conds->to_string();
}

std::ostream& operator<<(std::ostream& out, ref<Conds> conds) {
	return out << conds.get()->to_string();
}

} // NAMESPACE_END(cre)

