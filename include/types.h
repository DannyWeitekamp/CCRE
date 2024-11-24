#ifndef _CRE_TYPES_H_
#define _CRE_TYPES_H_

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

// Forward declarations
struct CRE_Context;
struct Type;
struct Item;
// struct FlagGroup;

// ------------------------------------------------------------
// : FlagGroup

struct FlagGroup {
    size_t builtin_flags;
    size_t builtin_flags_mask;
    HashMap<std::string, Item> custom_flags;

    FlagGroup(const HashMap<std::string, Item>& flags={});
    FlagGroup(const FlagGroup& flags);

    // FlagGroup(std::initializer_list<std::tuple<std::string, Item>> flags);

    void assign_flag(std::string_view key, Item value);
    void assign(const HashMap<std::string, Item>& flags);
    bool has_flags(const FlagGroup& other);
    
    
    // template <class ... Ts>
    // FlagGroup(Ts && ... inputs){
    //   // std::cout << std::endl;
    //   // Item items[sizeof...(Ts)];
    //   HashMap<std::string, Item> flags = {};
    //   flags.reserve(sizeof...(Ts));
    //   // std::vector<Item> items = {};
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
    std::vector<CRE_Type*> sub_types;
    CRE_Context* context;
    int32_t type_index;
    uint16_t t_id;
    uint8_t builtin;
    uint8_t kind;
    // size_t size; TODO!!!
    

    CRE_Type(std::string_view _name, 
        uint16_t _t_id,
        std::vector<CRE_Type*> _sub_types = {}, 
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

    uint64_t get_flag(uint64_t flag);
    // CRE_Type* get_type();
};

// Type declaration
struct FactType : public CRE_Type{
    std::vector<MemberSpec> members;
    uint64_t builtin_flags;
    HashMap<std::string, Item> flags;
    std::vector<Item> member_names_as_items;
    uint8_t finalized;

    FactType(std::string_view _name, 
         const std::vector<CRE_Type*>& _sub_types = {}, 
         const std::vector<MemberSpec>& _members = {},
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
uint64_t get_builtin_flag(uint64_t* flags, uint64_t flag_n);
int get_unique_id_index(FactType* type);

HashMap<std::string, Item> parse_builtin_flags(
    uint64_t* builtin_ptr,
    const HashMap<std::string, Item>& flags,
    bool as_mask=false);

CRE_Type* define_type(std::string_view name, 
                  const std::vector<CRE_Type*>& sub_type={},
                  CRE_Context* context = nullptr);

FactType* define_fact(std::string_view name, 
                  const std::vector<MemberSpec>& members,
                  const std::vector<CRE_Type*>& sub_types={},
                  const HashMap<std::string, Item>& flags={},
                  CRE_Context* context=nullptr 
                  ) ;

extern "C" size_t FactType_get_member_index(FactType* type, char* key);


 
// Global variable declarations
extern CRE_Type* cre_undefined;
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

extern std::vector<CRE_Type*> cre_builtins;

std::vector<CRE_Type*> make_builtins();
// extern CRE_Context default_context;


struct DefferedType : public CRE_Type {
    std::string name;
    std::vector<CRE_Type*> sub_types;
    uint16_t t_id;
    uint8_t builtin;

    DefferedType(std::string_view _name);    
};



#endif /* _CRE_TYPES_H_ */
