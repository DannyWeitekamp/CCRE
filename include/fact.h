#ifndef _CRE_FACT_H_
#define _CRE_FACT_H_

#include <vector>
#include <bit>
#include "../include/item.h"
#include "../include/cre_obj.h"
#include "../include/types.h"
#include "../include/context.h"
#include "../include/hash.h"
#include "../include/intern.h"

// Externally Defined Forward Declares
class FactSet;
// extern CRE_Context* current_context;

class Fact : public CRE_Obj {

public: 
  //  -- Members --
  FactType*     type;
  FactSet* 	parent; 
  uint32_t  f_id;
  uint32_t  length;
  uint64_t 	hash;
  void* 	pool;

  // For internal use;
  uint8_t cov_flag;
  // Whether or the fact is immutable;
  uint8_t immutable;
  uint8_t has_mutable;

  uint16_t pad1[2];

  //  -- Methods --
  Fact(void* type);
  Fact(uint32_t _length);

  void set_unsafe(uint32_t a_id, const Item& val);

  template<class T>
  void set(uint32_t a_id, const T& val){
    if(a_id >= length){
      throw std::out_of_range("Setting fact member beyond its length.");
    }

    // Convert to Item, always intern strings, always hash on assignment;
    Item item;
    if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>) {
      InternStr intern_str(intern(val));
      item = Item(intern_str);
    }else{
      uint64_t hash = CREHash{}(val); 
      item = Item(val);
      item.hash = hash;
    }

    

    // std::cout << "input " << std::to_string(a_id) << " = " << item << std::endl;
    // std::cout << "T:" << uint64_t(this->type) << std::endl;
    if(this->type != nullptr && a_id < this->type->members.size()){
      CRE_Type* mbr_type = this->type->members[a_id].type;

      if(item.t_id == T_ID_NULL){
        item.t_id = mbr_type->t_id;
      }else if(mbr_type->t_id != item.t_id && mbr_type->t_id != 0){
        CRE_Type* type = current_context->types[item.t_id-1];
        throw std::invalid_argument("Setting item[" + std::to_string(a_id) + "] with type '" + mbr_type->name + "' to value " \
              + item_to_string(item) + " with type '" + type->name + "'");
      }
    }

    // Pointer to the 0th Item of the FactHeader
    Item* data_ptr(std::bit_cast<Item*>(
        ((uint8_t*) this) + sizeof(Fact)
    ));
    data_ptr[a_id] = item;
  }



  Item* get(uint32_t a_id);
  std::vector<Item*> get_items();
  std::string to_string();
    
};

std::ostream& operator<<(std::ostream& out, Fact* fact);


extern "C" Fact* _alloc_fact(uint32_t _length);
extern "C" void _init_fact(Fact* fact, uint32_t _length, FactType* type=NULL);

extern "C" Fact* empty_fact(FactType* type);
extern "C" Fact* empty_untyped_fact(uint32_t _length);
extern "C" Fact* new_fact(FactType* type, const Item* items, size_t _length);
Fact* new_fact(FactType* type, const std::vector<Item>& items);


const uint8_t DEFAULT_VERBOSITY = 2;


extern "C" std::string fact_to_unique_id(Fact* fact);
extern "C" std::string fact_to_string(Fact* fact, uint8_t verbosity=DEFAULT_VERBOSITY);
extern "C" Item fact_to_item(Fact* fact);


template <class ... Ts>
Fact* make_fact(FactType* type, Ts && ... inputs){
  // std::cout << std::endl;
  Item items[sizeof...(Ts)];
  // std::vector<Item> items = {};
  // items.reserve();
  int i = 0;
  ([&]
    {
        // Do things in your "loop" lambda
        // Item item = to_item(inputs);
        // items.push_back(item);
        items[i] = to_item(inputs);
        ++i;
        
    } (), ...);
  return new_fact(type, items, i);
}


#endif /* _CRE_FACT_H_ */
