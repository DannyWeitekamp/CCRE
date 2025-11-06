
#include <stdint.h>              // for uint16_t
#include "../include/cre_obj.h"  // for CRE_Obj
#include "../include/t_ids.h"    // for T_ID_CONDS, T_ID_FACT, T_ID_FAC...

namespace cre {

bool CRE_Objs_equal(CRE_Obj* a, CRE_Obj* b){
    uint16_t a_t_id = a->get_t_id();
    uint16_t b_t_id = b->get_t_id();

    if(a_t_id != b_t_id){
        return false;
    }

    switch(a_t_id){
    case T_ID_FACT:
        return a == b;
    case T_ID_FACTSET:
        return a == b;
    case T_ID_VAR:
        return a == b;
    case T_ID_FUNC:
        return a == b;
    case T_ID_LITERAL:
        return a == b;
    case T_ID_CONDS:
        return a == b;
    case T_ID_RULE:
        return a == b;
    }
    return false;
}

} // NAMESPACE_END(cre)

