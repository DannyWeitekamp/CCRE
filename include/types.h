#pragma once

#include "../include/context.h"
#include "../include/t_ids.h"
#include "../include/hash.h"
#include "../include/cre_obj.h"
// #include "../include/item.h"

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
struct FactType;

template <class ... Ts>
ref<Fact> make_fact(FactType* type, const Ts& ... inputs);

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

typedef void (*DynamicDtor)(void*);


struct CRE_Type : CRE_Obj{
    std::string name;
    vector<CRE_Type*> sub_types;
    CRE_Context* context;
    DynamicDtor dynamic_dtor;
    float structure_weight = 1.0f;
    float match_weight = 1.0f;
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
        DynamicDtor dynamic_dtor = nullptr,
        CRE_Context* context = nullptr
    );    

    inline uint16_t get_t_id() const noexcept{
        return t_id;
    }

    std::string to_string();

    bool isa(const CRE_Type* obj) const;
    bool issubclass(const CRE_Type* obj) const;

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
struct FactType : public CRE_Type {
    vector<MemberSpec> members;
    uint64_t builtin_flags;
    HashMap<std::string, Item> flags;
    vector<Item> member_names_as_items;
    
    int16_t unique_id_index;
    uint8_t finalized;
    // uint8_t pad[5];

    FactType(std::string_view _name, 
         const vector<MemberSpec>& members = {},
         FactType* inherts_from = nullptr, 
         const HashMap<std::string, Item>& flags = {},
         CRE_Context* context=nullptr
    );

    int get_attr_index(std::string_view attr);
    std::string get_item_attr(int index);
    CRE_Type* get_item_type(int index);
    CRE_Type* get_attr_type(std::string_view name);
    void ensure_finalized();
    bool try_finalize();
    inline size_t size(){return members.size();}

    template <class ... Ts>
    ref<Fact> operator()(const Ts& ... inputs){
        return make_fact(this, inputs...);
    }

    // std::string to_string();

};

// Function declarations
std::string to_string(CRE_Type* value);
std::ostream& operator<<(std::ostream& outs, CRE_Type* type);


void set_builtin_flag(uint64_t* flags, uint64_t flag_n, uint64_t val);
uint64_t get_builtin_flag(const uint64_t* flags, uint64_t flag_n);
int get_unique_id_index(const vector<MemberSpec>& member_specs);
int get_unique_id_index(FactType* type);

HashMap<std::string, Item> parse_builtin_flags(
    uint64_t* builtin_ptr,
    const HashMap<std::string, Item>& flags,
    bool as_mask=false);

CRE_Type* define_type(std::string_view name, 
                  const vector<CRE_Type*>& sub_types={},
                  uint16_t byte_width = 0,
                  DynamicDtor dynamic_dtor = nullptr,
                  CRE_Context* context = nullptr);

FactType* define_fact(std::string_view name, 
                  const vector<MemberSpec>& members,
                  FactType* inherts_from = nullptr,
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
extern CRE_Type* cre_Logic;
extern CRE_Type* cre_Rule;

extern vector<CRE_Type*> cre_builtins;

inline CRE_Type* get_cre_type(uint16_t t_id) {
    return cre_builtins[t_id];
}

void ensure_builtins();

// vector<CRE_Type*> global_builtins;
// extern CRE_Context default_context;

class CRE_Obj;
class Fact;
class FactSet;
struct Var;
struct Func;
struct FuncRef;
struct Literal;
struct Logic;
struct Rule;
struct StrBlock;





template <typename T, auto ExceptF>
CRE_Type* to_cre_type() {
    // using T = std::remove_cvref_t<_T>;
    using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;

    if constexpr (std::is_same_v<bool, DecayT>) {
        return cre_bool;
    }else if constexpr (std::is_integral_v<DecayT>) {
        return cre_int;
    } else if constexpr (std::is_floating_point_v<DecayT>) {
        return cre_float;
    } else if constexpr (std::is_same_v<typename std::decay<T>::type, const char*> ||
                         std::is_same_v<typename std::decay<T>::type, char*> ||
                         std::is_same_v<std::string, DecayT> ||
                         std::is_same_v<std::string_view, DecayT> ||
                         std::is_same_v<StrBlock, DecayT>) {
        return cre_str;
    } else if constexpr (std::is_same_v<CRE_Obj, DecayT>){
        return cre_obj;
    } else if constexpr (std::is_same_v<Fact, DecayT>){
        return cre_Fact;
    } else if constexpr (std::is_same_v<FactSet, DecayT>){
        return cre_FactSet;
    } else if constexpr (std::is_same_v<Var, DecayT>){
        return cre_Var;
    } else if constexpr (std::is_same_v<Func, DecayT>){
        return cre_Func;
    } else if constexpr (std::is_same_v<Literal, DecayT>){
        return cre_Literal;
    } else if constexpr (std::is_same_v<Logic, DecayT>){
        return cre_Logic;  
    } else if constexpr (std::is_same_v<Rule, DecayT>){
        return cre_Rule;
    }else{
        return ExceptF();     
    }
}

inline CRE_Type* _unsupported_type(){
    throw std::runtime_error("Unsupported type for to_cre_type()");
    return nullptr;
}


template <typename T>
CRE_Type* to_cre_type() {
    return to_cre_type<T,_unsupported_type>();
}

inline CRE_Type* _return_null(){
    return nullptr;
}

template <typename T>
CRE_Type* to_cre_type_or_null() {
    return to_cre_type<T,_return_null>();

    // // using T = std::remove_cvref_t<_T>;
    // using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;

    // if constexpr (std::is_same_v<bool, DecayT>) {
    //     return cre_bool;
    // }else if constexpr (std::is_integral_v<DecayT>) {
    //     return cre_int;
    // } else if constexpr (std::is_floating_point_v<DecayT>) {
    //     return cre_float;
    // } else if constexpr (std::is_same_v<std::string, DecayT> ||
    //                      std::is_same_v<std::string_view, DecayT> ||
    //                      std::is_same_v<StrBlock, DecayT>) {
    //     return cre_str;
    // } else if constexpr (std::is_same_v<CRE_Obj, DecayT>){
    //     return cre_obj;
    // } else if constexpr (std::is_same_v<Fact, DecayT>){
    //     return cre_Fact;
    // } else if constexpr (std::is_same_v<FactSet, DecayT>){
    //     return cre_FactSet;
    // } else if constexpr (std::is_same_v<Var, DecayT>){
    //     return cre_Var;
    // } else if constexpr (std::is_same_v<Func, DecayT>){
    //     return cre_Func;
    // } else if constexpr (std::is_same_v<Literal, DecayT>){
    //     return cre_Literal;
    // } else if constexpr (std::is_same_v<Logic, DecayT>){
    //     return cre_Logic;
    // } else if constexpr (std::is_same_v<Rule, DecayT>){
    //     return cre_Rule;
    // }else{
    //     return (CRE_Type*) nullptr;
    // }   
}

template <typename T>
std::string type_name_helper() {
    if constexpr(std::is_same_v<T, Item>){
		return "cre::Item";
	}else{
		CRE_Type* cre_type = to_cre_type_or_null<T>();
		if(cre_type == nullptr){
			return typeid(T).name();
		}
		std::stringstream ss;
		ss << cre_type;
		return ss.str();
		
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
    is_var_or_func<
        std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>
    >::value || has_var_or_func<Rest...>::value
> {};


struct NoneType {
    // Truthiness of None is false
    explicit operator bool() const {
        return false;
    }
};

extern NoneType None;

std::ostream& operator<<(std::ostream& outs, const NoneType& none);


} // NAMESPACE_END(cre)
