#include "../include/mapping.h"

namespace cre {

void Mapping_dtor(const CRE_Obj* x){
	Mapping* mapping = (Mapping*) x;

    for(size_t i=0; i < mapping->length; i++){
        Item* item = &mapping->get(i);
        item->release();
    }

	CRE_Obj_dtor(x);
}

ref<Mapping> alloc_mapping(size_t length, AllocBuffer* buffer){
    auto [mapping_addr, did_malloc] = alloc_cre_obj(SIZEOF_MAPPING(length), &Mapping_dtor, T_ID_MAPPING, buffer);
  
    ref<Mapping> mapping = new (mapping_addr) Mapping(length);
  
    return mapping;
}
ref<Mapping> new_mapping(Item* values, size_t length,
                         ref<CRE_Obj> host_obj, VarMapType* var_map,
                         AllocBuffer* buffer){
    ref<Mapping> mapping = alloc_mapping(length, buffer);
    for(size_t i=0; i < length; i++){
        mapping->set_init(i, values[i]);
    }
    mapping->host_obj = host_obj;
    mapping->var_map = var_map;
    return mapping;
}

}