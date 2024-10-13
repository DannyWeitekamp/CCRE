#ifndef _CRE_CONTEXT_H_
#define _CRE_CONTEXT_H_


#include "../include/types.h"
#include "../include/hash.h"



// CRE_Context declaration
struct CRE_Context {
    std::string name;
    std::vector<CRE_Type*> types;
    // std::unordered_map<std::string, uint16_t> type_name_map;
    HashMap<std::string, uint16_t> type_name_map = {};

    CRE_Context(std::string _name);
    size_t _add_type(CRE_Type* t);
    CRE_Type* get_type(const std::string_view& name);
    FactType* get_fact_type(const std::string_view& name);
};

extern "C" CRE_Context* CRE_set_current_context(CRE_Context* context);

extern CRE_Context* default_context;
extern CRE_Context* current_context;

#endif /* _CRE_CONTEXT_H_ */
