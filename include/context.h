#pragma once 


#include "../include/ref.h"
// #include "../include/types.h"
#include "../include/hash.h"
#include "../include/pool_allocator.h"
// #include "../include/control_block.h"
// #include "../include/intern.h"

namespace cre {



// Forward Declare
struct InternStr;
struct CRE_Type;
struct FactType;
// struct ControlBlock;

// CRE_Context declaration
struct CRE_Context {
    std::string name;
    // std::vector<CRE_Type*> cre_builtins = {};
    std::vector<CRE_Type*> types = {};
    std::vector<CRE_Type*> overwritten_types = {};
    // std::unordered_map<std::string, uint16_t> type_name_map;
    HashMap<std::string, uint16_t> type_name_map = {};

    // Note, is a set of std::string_view instead of std::string
    //  because std::string will re-allocate data on copy
    HashSet<std::string_view> intern_set;

public: 
    CRE_Context(std::string _name);
    size_t _add_type(CRE_Type* t);
    CRE_Type* _get_type(std::string_view name) noexcept;
    CRE_Type* get_type(std::string_view name);
    FactType* _get_fact_type(std::string_view name) noexcept;
    FactType* get_fact_type(std::string_view name);
    std::string to_string();

    InternStr intern(std::string_view sv) noexcept;

    ~CRE_Context();
};




CRE_Context* get_context(std::string_view context);
CRE_Context* set_current_context(CRE_Context* context);
CRE_Context* set_current_context(std::string_view context);


extern HashMap<std::string, std::unique_ptr<CRE_Context>>& __all_CRE_contexts;
extern CRE_Context* default_context;
extern CRE_Context* current_context;


// -- Schwarzt Counter To Gate Initialization / Teardown of Context--
extern PoolAllocator<ControlBlock>& global_cb_pool; 
static struct ContextInitializer {
  ContextInitializer ();
  ~ContextInitializer ();
} dc_pool_initializer;
// END -- Schwarzt Counter To Initialization / Teardown of Context--


} // NAMESPACE_END(cre)
