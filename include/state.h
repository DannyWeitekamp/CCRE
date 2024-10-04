#ifndef _State_H_
#define _State_H_

#include <string>
#include <vector>
#include <bit>
#include <cstdint>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include "item.h"
#include "types.h"


using namespace std;


// Forward declarations
struct EmptyBlock;
struct FactHeader;
struct State;
struct Iterator;

struct Block{
  uint64_t data[3];
};

// Function declarations
std::string string_format(const std::string& format, ...);
FactHeader* alloc_fact_block(State* state, size_t size, bool recycle_blocks=true);
uint32_t empty_fact_header(State* state, uint32_t length);
uint32_t empty_fact_header(uint32_t length);
bool _contains_header(State* state, FactHeader* fact);
void _retract_header(State* state, FactHeader* _fact);
State* get_state(FactHeader* fact);

struct FactHeader {
  uint32_t f_id;
  uint16_t pad0[2];
  uint64_t hash;
  uint16_t t_id;
  uint16_t pad1[1];
  uint32_t length;

  template<class T>
  void set(uint32_t a_id, T val){
    if(a_id >= length){
      throw out_of_range("Setting fact member beyond its length.");
    }
    Item item = to_item(val);

    // Pointer to the 0th Item of the FactHeader
    Item* data_ptr = bit_cast<Item*>(
        bit_cast<uint64_t>(this) + sizeof(FactHeader)
    );
    data_ptr[a_id] = item;
    // uint64_t item_ptr = this_ptr + sizeof(FactHeader) + a_id * sizeof(Item);
    // cout << this_ptr << "," << item_ptr << "\n";
    // Item* ptr = bit_cast<Item*>()
    // memcpy()
    // *ptr = item;
  }
  

  Item* get(uint32_t a_id);
  FactHeader(uint32_t _length);
};



struct State {
  size_t                       size;
  //unordered_map<Item, FactHeader> indexer;
  int64_t                      first_empty_f_id;
  // uint32_t                     last_empty_f_id;
  void*                        meta_data;
  FactHeader*                        data;
  FactHeader*                        head;

  State(size_t _size=1);
  ~State();
  void print_layout();
  void print_empties();

  struct Iterator  {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = FactHeader;
        using pointer           = FactHeader*;
        using reference         = FactHeader&;

        Iterator(FactHeader* ptr) : f_ptr(ptr) {}

        reference operator*() const { return *f_ptr; }
        FactHeader* operator->() { return f_ptr; }
        Iterator& operator++() {
          FactHeader fact = *f_ptr;
          do {
            // cout << "F_PTR: " << bit_cast<uint64_t>(f_ptr) << endl;
            // cout << "LENGTH: " << f_ptr->length << endl;
            f_ptr += 1+f_ptr->length;
          
            fact = *f_ptr;
          }
          while (fact.t_id == T_ID_EMPTY_BLOCK);
            //&& bit_cast<uint64_t>(f_ptr) < this->end());
          return *this;
        };
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; };

        friend bool operator== (const Iterator& a, const Iterator& b) { return a.f_ptr == b.f_ptr; };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return a.f_ptr != b.f_ptr; };  
    

      private:
        FactHeader* f_ptr;

    };

    Iterator begin() { return Iterator(data+1); }
    Iterator end()   { return Iterator(head); }
};

// class Iterator {
//   public:
//       using iterator_category = std::forward_iterator_tag;
//       using difference_type   = std::ptrdiff_t;
//       using value_type        = FactHeader;
//       using pointer           = FactHeader*;
//       using reference         = FactHeader&;

//       Iterator(FactHeader* ptr);

//       reference operator*() const;
//       pointer operator->();
//       Iterator& operator++();
//       Iterator operator++(int);

//       friend bool operator== (const Iterator& a, const Iterator& b);
//       friend bool operator!= (const Iterator& a, const Iterator& b);
// };

struct FactView {
    State* state;
    uint32_t f_id;
    uint32_t length;

    template<class T>
    void set(uint32_t a_id, T val){
      FactHeader* header = &state->data[f_id];
      return header->set(a_id, val);
    }

    Item* get(uint32_t a_id);
    FactView(State* _state, uint32_t _f_id, uint32_t _length);
    FactHeader* header_ptr();
};

// Function declarations
std::vector<Item*> fact_get_Items(FactHeader& fact);
std::string fact_to_string(FactHeader& fact);
FactView empty_fact(State* state, uint32_t length);
// FactView empty_fact(uint32_t length);
FactView declare(State* state, FactView fact_view);
bool retract(State* state, FactView fact_view);
bool retract(State* state, uint32_t f_id);
FactHeader* get(State* state, uint32_t f_id);

#endif /* _State_H_ */
