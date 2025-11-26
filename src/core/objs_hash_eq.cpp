
#include <stdint.h>              // for uint16_t
#include "../include/cre_obj.h"  // for CRE_Obj
#include "../include/t_ids.h"    // for T_ID_LOGIC, T_ID_FACT, T_ID_FAC...
#include "../include/fact.h"     // for Fact
// #include "../include/fact_set.h" // for FactSet
#include "../include/var.h"      // for Var
#include "../include/func.h"     // for Func
#include "../include/literal.h"  // for Literal
#include "../include/logic.h"    // for Logic
// #include "../include/rule.h"     // for Rule

namespace cre {

bool CRE_Objs_equal(const CRE_Obj* a, const CRE_Obj* b, bool semantic){
    uint16_t a_t_id = a->get_t_id();
    uint16_t b_t_id = b->get_t_id();

    if(a_t_id != b_t_id){
        return false;
    }

    switch(a_t_id){
    case T_ID_FACT:
        return facts_equal((Fact*) a, (Fact*) b, semantic);
    case T_ID_FACTSET:
        // TODO
        return a == b;
        // return *((FactSet*) a) == *((FactSet*) b);
    case T_ID_VAR:
        return vars_equal((Var*) a, (Var*) b, true, semantic);
    case T_ID_FUNC:
        return funcs_equal((Func*) a, (Func*) b, semantic);
    case T_ID_LITERAL:
        return literals_equal((Literal*) a, (Literal*) b, semantic);
    case T_ID_LOGIC:
        // TODO
        return a == b;
    case T_ID_RULE:
        // TODO
        return a == b;
        // return *((Rule*) a) == *((Rule*) b);
    }
    return false;
}

} // NAMESPACE_END(cre)

