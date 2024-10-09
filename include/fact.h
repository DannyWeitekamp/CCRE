#ifndef _CRE_FACT_H_
#define _CRE_FACT_H_

#include <vector>
#include "../include/item.h"
#include "../include/cre_obj.h"
#include "../include/types.h"

// Externally Defined Forward Declares
class FactSet;

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

  uint16_t pad1[3];

  //  -- Methods --
  Fact(void* type);
  Fact(uint32_t _length);

  template<class T>
  void set(uint32_t a_id, T val){
    if(a_id >= length){
      throw std::out_of_range("Setting fact member beyond its length.");
    }
    Item item = to_item(val);

    // Pointer to the 0th Item of the FactHeader
    Item* data_ptr = std::bit_cast<Item*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
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

extern "C" std::string fact_to_string(Fact* fact);
extern "C" Item fact_to_item(Fact* fact);


#endif /* _CRE_FACT_H_ */
