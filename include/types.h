#pragma once

#include "../include/t_ids.h"
#include "../include/hash.h"
#include "../include/cre_obj.h"
#include "../include/item.h"
// #include "../include/context.h"
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <sstream>


using std::cout;
using std::endl;
using std::vector;

namespace cre {

// Forward declarations
struct CRE_Context;
struct Item;

// ------------------------------------------------------------
// : FlagGroup

struct FlagGroup {
    uint64_t builtin_flags;
    uint64_t builtin_flags_mask;
    HashMap<std::string, Item> custom_flags;

// public:

    FlagGroup(const HashMap<std::string, Item>& flags={});
    // FlagGroup(const FlagGroup& flags);

    // FlagGroup(std::initializer_list<std::tuple<std::string, Item>> flags);

    void assign_flag(std::string_view key, Item value);
    void assign(const HashMap<std::string, Item>& flags);
    Item get_flag(std::string_view key) const;
    bool has_flags(const FlagGroup& other) const;
    
    
    // template <class ... Ts>
    // FlagGroup(Ts && ... inputs){
    //   // std::cout << std::endl;
    //   // Item items[sizeof...(Ts)];
    //   HashMap<std::string, Item> flags = {};
    //   flags.reserve(sizeof...(Ts));
    //   // vector<Item> items = {};
    //   // items.reserve();
    //   // int i = 0;
    //   ([&]
    //     {
    //         std::pair<std::string, typeof()>
    //         flags[inputs.first] = to_item(inputs.second);
    //         // Do things in your "loop" lambda
    //         // Item item = to_item(inputs);
    //         // items.push_back(item);
    //         // items[i] = to_item(inputs);
    //         // ++i;
            
    //     } (), ...);

    //   this->assign(flags);
    //   // return ::FlagGroup(flags);//new_fact(type, items, i);
    // }
    

    
};

// -------------------------------------------------------------
// CRE_Type

// // Alias CRE_dtor_function as pointer to void(void*)
// typedef void (*CRE_dtor_function)(CRE_Obj* ptr);

const uint8_t TYPE_KIND_BUILTIN = 1;
const uint8_t TYPE_KIND_FACT = 2;
const uint8_t TYPE_KIND_DEFFERED = 3;


struct CRE_Type : CRE_Obj{
    std::string name;
    vector<CRE_Type*> sub_types;
    CRE_Context* context;
    int32_t type_index;
    uint16_t t_id;
    uint16_t byte_width;
    uint8_t builtin;
    uint8_t kind;
    // size_t size; TODO!!!
    

    CRE_Type(std::string_view _name, 
        uint16_t _t_id,
        uint16_t byte_width,
        vector<CRE_Type*> _sub_types = {},
        uint8_t _builtin = 0,
        CRE_Context* context = nullptr
    );    

    // ~CRE_Type();
};

const uint64_t BIFLG_UNIQUE_ID =  0;
const uint64_t BIFLG_VISIBLE_ID = 1;
const uint64_t BIFLG_SEMANTIC_ID = 2;
const uint64_t BIFLG_VERBOSITY = 8;


struct MemberSpec {
    std::string name;
    CRE_Type* type; 
    FlagGroup flags;
    // uint64_t builtin_flags;
    // HashMap<std::string, Item> custom_flags;

    MemberSpec(std::string_view _name,
             CRE_Type* _type,
             const HashMap<std::string, Item>& _flags={}
    );

    MemberSpec(
            std::string_view _name,
            std::string_view _type_name,
            const HashMap<std::string, Item>& _flags
    );

    uint64_t get_flag(uint64_t flag) const;
    // CRE_Type* get_type();
};

// Type declaration
struct FactType : public CRE_Type{
    vector<MemberSpec> members;
    uint64_t builtin_flags;
    HashMap<std::string, Item> flags;
    vector<Item> member_names_as_items;
    int16_t unique_id_index;
    uint8_t finalized;
    // uint8_t pad[5];

    FactType(std::string_view _name, 
         const vector<CRE_Type*>& _sub_types = {}, 
         const vector<MemberSpec>& _members = {},
         const HashMap<std::string, Item>& flags = {},
         CRE_Context* context=nullptr
    );

    int get_attr_index(const std::string_view& attr);
    std::string get_item_attr(int index);
    CRE_Type* get_item_type(int index);
    CRE_Type* get_attr_type(const std::string_view& name);
    void ensure_finalized();
    bool try_finalized();
    inline size_t size(){return members.size();}

};

// Function declarations
std::string to_string(const CRE_Type* value);
std::ostream& operator<<(std::ostream& outs, const CRE_Type* type);


void set_builtin_flag(uint64_t* flags, uint64_t flag_n, uint64_t val);
uint64_t get_builtin_flag(const uint64_t* flags, uint64_t flag_n);
int get_unique_id_index(const vector<MemberSpec>& member_specs);
int get_unique_id_index(FactType* type);

HashMap<std::string, Item> parse_builtin_flags(
    uint64_t* builtin_ptr,
    const HashMap<std::string, Item>& flags,
    bool as_mask=false);

CRE_Type* define_type(std::string_view name, 
                  const vector<CRE_Type*>& sub_type={},
                  uint16_t byte_width = 0,
                  CRE_Context* context = nullptr);

FactType* define_fact(std::string_view name, 
                  const vector<MemberSpec>& members,
                  const vector<CRE_Type*>& sub_types={},
                  const HashMap<std::string, Item>& flags={},
                  CRE_Context* context=nullptr 
                  ) ;

extern "C" size_t FactType_get_member_index(FactType* type, char* key);


 
// Global variable declarations
extern CRE_Type* cre_undef;
extern CRE_Type* cre_none;
extern CRE_Type* cre_bool;
extern CRE_Type* cre_int;
extern CRE_Type* cre_float;
extern CRE_Type* cre_str;
extern CRE_Type* cre_obj;
extern CRE_Type* cre_Fact;
extern CRE_Type* cre_FactSet;
extern CRE_Type* cre_Var;
extern CRE_Type* cre_Func;
extern CRE_Type* cre_Literal;
extern CRE_Type* cre_Conditions;
extern CRE_Type* cre_Rule;

extern vector<CRE_Type*> cre_builtins;

vector<CRE_Type*> make_builtins();
// extern CRE_Context default_context;

class CRE_Obj;
class Fact;
class FactSet;
struct Var;
struct Func;
struct FuncRef;
struct Literal;
struct Conditions;
struct Rule;

template <typename _T>
CRE_Type* to_cre_type() {

    using T = std::remove_cvref_t<_T>;


    if constexpr (std::is_same_v<bool, T>) {
        return cre_bool;
    }else if constexpr (std::is_integral_v<T>) {
        return cre_int;
    } else if constexpr (std::is_floating_point_v<T>) {
        return cre_float;
    } else if constexpr (std::is_same_v<std::string, T> ||
                         std::is_same_v<std::string_view, T>) {
        return cre_str;
    } else if constexpr (std::is_same_v<CRE_Obj, T>){
        return cre_obj;
    } else if constexpr (std::is_same_v<Fact, T>){
        return cre_Fact;
    } else if constexpr (std::is_same_v<FactSet, T>){
        return cre_FactSet;
    } else if constexpr (std::is_same_v<Var, T>){
        return cre_Var;
    } else if constexpr (std::is_same_v<Func, T>){
        return cre_Func;
    } else if constexpr (std::is_same_v<Literal, T>){
        return cre_Literal;
    } else if constexpr (std::is_same_v<Conditions, T>){
        return cre_Conditions;
    } else if constexpr (std::is_same_v<Rule, T>){
        return cre_Rule;
    }
}


struct DefferedType : public CRE_Type {
    // std::string name;
    // vector<CRE_Type*> sub_types;
    // uint16_t t_id;
    // uint16_t byte_width;
    // uint8_t builtin;

    DefferedType(std::string_view _name);    
};

// -------------------------
// : Define is_var_or_func() and has_var_or_func()

template<typename T>
struct is_var_or_func : std::false_type {};

template<>
struct is_var_or_func<Var> : std::true_type {};

template<>
struct is_var_or_func<Func> : std::true_type {};

template<>
struct is_var_or_func<ref<Var>> : std::true_type {};

template<>
struct is_var_or_func<ref<Func>> : std::true_type {};

template<>
struct is_var_or_func<FuncRef> : std::true_type {};

// Helper to check if any argument is Var or Func
template<typename... Ts>
struct has_var_or_func : std::false_type {};

template<typename T, typename... Rest>
struct has_var_or_func<T, Rest...> : std::bool_constant<
    is_var_or_func<std::remove_cvref_t<T>>::value || has_var_or_func<Rest...>::value
> {};

} // NAMESPACE_END(cre)
