#pragma once
#include "../include/cre_obj.h"
#include "../include/t_ids.h"
// #include "../include/item.h"
#include "../include/var.h"
// #include "../include/ref.h"
// #include "../include/var_inds.h"
// #include "../include/literal.h"
// #include "../include/logic.h"
// #include "../include/alloc_buffer.h"
#include <vector>
#include <map>
#include <assert.h>


namespace cre {

struct VarInfo {
    Var* var;
    size_t pos=-1;
    size_t kind_pos=-1;
    size_t first_item=-1;
    std::vector<size_t> item_inds = {};
    uint8_t kind;

    VarInfo(Var* var, uint8_t kind, size_t first_item) :
        var(var), kind(kind), first_item(first_item) {}
};

struct FastAliasComparator {
    bool operator()(const Item& alias1, const Item& alias2) const {
        uint16_t t_id1 = alias1.get_t_id();
        uint16_t t_id2 = alias2.get_t_id();
        if(t_id1 != t_id2){
            return t_id1 < t_id2;
        }

        assert(t_id1 == T_ID_STR || t_id1 == T_ID_INT);
        assert(t_id2 == T_ID_STR || t_id2 == T_ID_INT);

        if(t_id1 == T_ID_STR){
            if(alias1.is_interned() && alias2.is_interned()){
                // Note: this is correct ordering need only be consistent over lifetime of the program
                return alias1.val < alias2.val;
            }
            return alias1.as<std::string_view>() < alias2.as<std::string_view>();
        }else if(t_id1 == T_ID_INT){
            return alias1.val < alias2.val;
        }
        return alias1.val < alias2.val;
    }
};
using VarMapType = std::map<Item, VarInfo, FastAliasComparator>;



void Mapping_dtor(const CRE_Obj* x);

struct Mapping : public CRE_Obj {
	static constexpr uint16_t T_ID = T_ID_MAPPING;
	// -- Members --
    ref<CRE_Obj> host_obj = nullptr;
    VarMapType* var_map = nullptr;
    size_t length = 0;

    Mapping(size_t length) :
        length(length)
    {}

    size_t size() const {return length;}

    

    inline void set(size_t index, Item value){
        if(index > 0 && index >= length){
            throw std::invalid_argument("Index out of range in Mapping::set");
        }
        // cout << "SET: " << index << ", " << length << endl;
        Item* data_ptr = (Item*)((uint8_t*) this + sizeof(Mapping));
        data_ptr[index] = value;
    }
    inline void set_init(size_t index, Item value){
        Item* data_ptr = (Item*)((uint8_t*) this + sizeof(Mapping));
        new (data_ptr + index) Item(value);
    }



    inline Item& get(size_t index) const {
        if(index > 0 && index >= length){
            throw std::invalid_argument("Index out of range in Mapping::get");
        }
        Item* data_ptr = (Item*)((uint8_t*) this + sizeof(Mapping));
        return data_ptr[index];
    }

    inline Item& operator[](size_t index) const {
        return get(index);
    }

    inline int16_t get_index(Item item) const {
        if(var_map == nullptr) return -1;
        auto it = var_map->find(item);
        if(it != var_map->end()){
            return it->second.pos;
        }
        return -1;
    }

    inline Item get(Item item) const {
        if(var_map == nullptr) return Item();
        auto it = var_map->find(item);
        if(it != var_map->end()){
            return get(it->second.pos);
        }
        return Item();
    }

    ref<Mapping> copy(AllocBuffer* buffer) const;
};





// ------------------------------------------------------------
// : SIZEOF_MAPPING(n)

const uint64_t MAPPING_ALIGN = alignof(Mapping);
// const uint64_t FACT_ALIGN = 64;

constexpr bool MAPPING_ALIGN_IS_POW2 = (MAPPING_ALIGN & (MAPPING_ALIGN - 1)) == 0;
#define _SIZEOF_MAPPING(n) (sizeof(Mapping)+(n)*sizeof(Item))

#if MAPPING_ALIGN_IS_POW2 == true
  #define MAPPING_ALIGN_PADDING(n_bytes) ((MAPPING_ALIGN - (n_bytes & (MAPPING_ALIGN-1))) & (MAPPING_ALIGN-1))
#else
  #define MAPPING_ALIGN_PADDING(n_bytes) ((MAPPING_ALIGN - (n_bytes % (MAPPING_ALIGN))) % (MAPPING_ALIGN))
#endif

constexpr bool MAPPING_NEED_ALIGN_PAD = ((_SIZEOF_MAPPING(0) % MAPPING_ALIGN) | (sizeof(Item) % MAPPING_ALIGN)) != 0;

#if MAPPING_NEED_ALIGN_PAD
#define SIZEOF_MAPPING(n) (_SIZEOF_MAPPING(n) + MAPPING_ALIGN_PADDING(_SIZEOF_MAPPING(n)))
#else
#define SIZEOF_MAPPING(n) _SIZEOF_MAPPING(n)
#endif


ref<Mapping> alloc_mapping(size_t length=0, AllocBuffer* buffer=nullptr);
ref<Mapping> new_mapping(Item* values, size_t length=0,
     ref<CRE_Obj> host_obj=nullptr, VarMapType* var_map=nullptr,
     AllocBuffer* buffer=nullptr
);

ref<Mapping> new_mapping(size_t length=0,
    ref<CRE_Obj> host_obj=nullptr, VarMapType* var_map=nullptr,
    AllocBuffer* buffer=nullptr
);


template<typename T>
inline ref<Mapping> new_mapping(const std::vector<T>& values,
                                 ref<CRE_Obj> host_obj=nullptr, VarMapType* var_map=nullptr,
                                 AllocBuffer* buffer=nullptr){

    Item* values_ptr = (Item*) alloca(values.size() * sizeof(Item));
    for(size_t i=0; i<values.size(); i++){
        new (values_ptr + i) Item(values[i]);
    }
    return new_mapping(values_ptr, values.size(), host_obj, var_map, buffer);
}

} // namespace cre