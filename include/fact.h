#ifndef _CRE_FACT_H_
#define _CRE_FACT_H_

#include <vector>
#include "../include/item.h"
#include "../include/cre_obj.h"

using namespace std;

// Externally Defined Forward Declares
class FactSet;

class Fact : public CRE_Obj {

public: 
  //  -- Members --
  void*     type;
  void* 	pool;
  FactSet* 	parent; 
  uint64_t 	hash;
  uint32_t  f_id;
  uint32_t  length;

  // For internal use;
  uint8_t cov_flag;
  // Whether or the fact is immutable;
  uint8_t immutable;

  uint16_t pad1[1];

  //  -- Methods --
  Fact(void* type);
  Fact(uint32_t _length);

  template<class T>
  void set(uint32_t a_id, T val){
    if(a_id >= length){
      throw out_of_range("Setting fact member beyond its length.");
    }
    Item item = to_item(val);

    // Pointer to the 0th Item of the FactHeader
    Item* data_ptr = bit_cast<Item*>(
        bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    data_ptr[a_id] = item;
  }

  Item* get(uint32_t a_id);
  vector<Item*> get_items();
  string to_string();
    
};


extern "C" Fact* alloc_fact(uint32_t _length);
extern "C" Fact* new_fact(uint32_t _length);
extern "C" string fact_to_string(Fact* fact);

#endif /* _CRE_FACT_H_ */
