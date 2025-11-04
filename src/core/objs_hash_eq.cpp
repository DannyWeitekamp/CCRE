
#include "../include/cre_obj.h"
#include "../include/t_ids.h"

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
    case T_ID_CONDITIONS:
        return a == b;
    case T_ID_RULE:
        return a == b;
    }
    return false;
}

} // NAMESPACE_END(cre)

