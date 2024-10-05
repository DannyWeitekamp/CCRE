#include <vector>
#include <bit>
#include <cstdint>
#include <sstream> 
#include <cstring>
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/item.h"


using namespace std;

Item* Fact::get(uint32_t a_id){
    Item* data_ptr = bit_cast<Item*>(
        bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return &data_ptr[a_id];
}


extern "C" Fact* alloc_fact(uint32_t _length){
	void* ptr = malloc(sizeof(Fact) + _length * sizeof(Item));
	return (Fact*) ptr;
}

extern "C" Fact* new_fact(uint32_t _length){
	Fact* fact = alloc_fact(_length);
	fact->f_id = 0;
	fact->hash = 0;
	fact->length = _length;
	fact->parent = (FactSet*) NULL;
	return fact;
}


vector<Item*> Fact::get_items(){
  vector<Item*> out;
  out.reserve(this->length);
  for(int i=0; i < this->length; i++){
    Item* item = this->get(i);
    out.push_back(item);
  }
  return out;
}

extern "C" string fact_to_string(Fact* fact){
  stringstream ss;
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


