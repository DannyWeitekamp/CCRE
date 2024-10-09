#include <vector>
#include <bit>
#include <cstdint>
#include <sstream> 
#include <cstring>
#include <iostream>
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/item.h"


Item* Fact::get(uint32_t a_id){
    Item* data_ptr = std::bit_cast<Item*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return &data_ptr[a_id];
}


extern "C" Fact* _alloc_fact(uint32_t _length){
	Fact* ptr = (Fact*) malloc(sizeof(Fact) + _length * sizeof(Item));
	return ptr;
}

extern "C" void _init_fact(Fact* fact, uint32_t _length, FactType* type){
	fact->type = type;
	fact->f_id = 0;
	fact->hash = 0;
	fact->length = _length;
	fact->parent = (FactSet*) NULL;
}

extern "C" Fact* empty_fact(FactType* type){
	uint32_t _length = (uint32_t) type->members.size();
	Fact* fact = _alloc_fact(_length);
	_init_fact(fact, _length, type);
	return fact;
}

extern "C" Fact* empty_untyped_fact(uint32_t _length){
	Fact* fact = _alloc_fact(_length);
	_init_fact(fact, _length, NULL);
	return fact;
}

extern "C" Fact* new_fact(FactType* type, const Item* items, size_t _length){
	uint32_t length;

	if(type == NULL){
		length = _length;
	}else{
		length = std::max(type->members.size(), _length);
	}

	Fact* fact = _alloc_fact(length);
	_init_fact(fact, uint32_t(length), type);
	
	memcpy(((uint8_t*)fact) + sizeof(Fact), (uint8_t*) items, length * sizeof(Item));
	return fact;
}

Fact* new_fact(FactType* type, const std::vector<Item>& items){
	return new_fact(type, items.data(), items.size());
}


std::vector<Item*> Fact::get_items(){
  std::vector<Item*> out;
  out.reserve(this->length);
  for(int i=0; i < this->length; i++){
    Item* item = this->get(i);
    out.push_back(item);
  }
  return out;
}

extern "C" std::string fact_to_string(Fact* fact){
  std::stringstream ss;
  ss << "Fact(";  

  // vector<Item> items = fact_get_items(fact);
  size_t L = fact->length;
  for(int i=0; i < L; i++){
    Item* item = fact->get(i);
    ss << repr_item(*item);
    if(i != L-1){
      ss << ", ";  
    }    
  }
  ss << ")";  
  return ss.str();
}

string Fact::to_string(){
	return fact_to_string(this);
}

extern "C" void fact_dtor(Fact* fact){
	for(size_t i=0; i < fact->length; i++){
		Item* item = fact->get(i);
		if(item->t_id == T_ID_OBJ){
			ObjItem* o_item = (ObjItem*) item;
			CRE_decref((CRE_Obj*) o_item->data);	
		}
	}
	if(fact->alloc_buffer != (void *) NULL){
		CRE_decref((CRE_Obj*) fact->alloc_buffer);
	}
	free(fact);
}


std::ostream& operator<<(std::ostream& out, Fact* fact){
	return out << fact_to_string(fact);
}


Item to_item(Fact* arg) { return fact_to_item(arg);}

extern "C" Item fact_to_item(Fact* fact) {
    Item item;
    item.val = std::bit_cast<uint64_t>(fact);
    item.hash = fact->hash;
    item.t_id = T_ID_FACT;
    return item;
}
