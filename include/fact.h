#ifndef _CRE_FACT_H_
#define _CRE_FACT_H_

#include <vector>
#include <bit>
#include "../include/alloc_buffer.h"
#include "../include/item.h"
#include "../include/cre_obj.h"
#include "../include/types.h"
#include "../include/context.h"
#include "../include/hash.h"
#include "../include/intern.h"

// Externally Defined Forward Declares
struct FactSet;
struct AllocBuffer;
// extern CRE_Context* current_context;

struct Fact : public CRE_Obj {

public: 
  //  -- Members --
  FactType*     type;
  FactSet* 	parent; 
  uint32_t  f_id;
  uint32_t  length;
  uint64_t 	hash;
  AllocBuffer* 	alloc_buffer;

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

  // Executes type specific .set()
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

    // Handle type checking against the fact's type   
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

    // Get pointer to the 0th Item
    Item* data_ptr(std::bit_cast<Item*>(
        ((uint8_t*) this) + sizeof(Fact)
    ));
    // Assign to the a_id'th item
    data_ptr[a_id] = item;
  }


  Item* get(uint32_t a_id) const;
  std::vector<Item*> get_items() const;
  std::string to_string();
  inline size_t size() const;

  Fact* slice_into(AllocBuffer& buffer, int start, int end, bool deep_copy);
  Fact* slice(int start, int end, bool deep_copy);
  Fact* copy_into(AllocBuffer& buffer, bool deep_copy);
  Fact* copy(bool deep_copy);

  bool operator==(const Fact& other) const;
  


  class Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Item;
    using pointer           = Item*;
    using reference         = Item&;

    private:
        Item* current;
    public:
        explicit Iterator(Item* item_ptr) : current(item_ptr) {}        
        reference operator*() const { return *current; }
        pointer operator->() const { return &*current; }
        Iterator& operator++() { ++current; return *this; }
        bool operator!=(const Iterator& other) const { return current != other.current; }
        bool operator==(const Iterator& other) const { return current == other.current; }
  };

  Iterator begin() const {
    uint8_t* data = (uint8_t*) this;
    return Iterator((Item*) (data + sizeof(Fact)));
  }
  Iterator end() const {
    uint8_t* data = (uint8_t*) this;
    return Iterator((Item*) (data + sizeof(Fact) + this->length*sizeof(Item)));
  }

  Item operator[](size_t index) {
    return this->get(index);
  }
};

// Enable Iteration over Fact* 
Fact::Iterator begin(const Fact* fact);
Fact::Iterator end(const Fact* fact);



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


//-------------------------------------------------------------
struct FactView {
    // -- Members --
    Fact* fact;
    uint16_t start;
    uint16_t end_;


    // -- Methods --
    explicit FactView(Fact* _fact, int _start, int _end) : fact(_fact) {
        start = uint16_t(_start >= 0 ? _start : fact->length + _start);
        end_ = uint16_t(_end >= 0 ? _end : fact->length + _end);

      // #ifndef NDEBUG
        if (end_ < start ||
            start >= fact->length ||
            end_ > fact->length) {
            throw std::out_of_range("Invalid slice range");
        }
      // #endif
    }

    // Iterator class for the slice
    class Iterator {
    private:
        typename Fact::Iterator it;
    public:
        explicit Iterator(const Fact::Iterator& iter) : it(iter) {}
        Iterator& operator++() { ++it; return *this; }
        bool operator!=(const Iterator& other) const { return it != other.it; }
        auto& operator*() const { return *it; }
    };

    // Begin and end methods for range-based for loops
    Iterator begin() {
      return Iterator(Fact::Iterator(&*fact->begin() + start)); 
    };

    Iterator end() { 
      return Iterator(Fact::Iterator(&*fact->begin() + end_)); 
    };

    Item* get(size_t index) {
      if (index >= end_ - start) {
          throw std::out_of_range("Index out of slice range");
      }
      return fact->get(start + index);
    }

    // Subscript operator for direct access
    Item operator[](size_t index) {
      return *this->get(index);
    }

    // Size of the view
    inline size_t size() const { return end_-start; }

    bool operator==(const FactView& other) const;
    
};


//--------------------------------------------------------------
// : FactSlice


// template<int Start, int End>
// class FactSlice {
// private:
//     Fact* fact;

// public:
//     static int64_t _get_size(size_t length){
//       if constexpr ((Start >= 0) == (End >= 0)){
//         // std::cout << "THIS ONE: " << Start <<  ", " << End << std::endl;
//         return End - Start;   
//       }else{
//         int start = Start >= 0 ? Start : length + Start;
//         int end = End >= 0 ? End : length + End;
//         // std::cout << "THAT ONE: " << start << ", " << end << std::endl;
//         return end-start;
//       }
//     }

//     explicit FactSlice(Fact* _fact) : fact(_fact) {
//         if (_get_size(fact->length) < 0) {
//             throw std::out_of_range("Invalid slice range");
//         }
//     }

//     // Iterator class for the slice
//     class Iterator {
//     private:
//         typename Fact::Iterator it;
//     public:
//         explicit Iterator(const Fact::Iterator& iter) : it(iter) {}
//         Iterator& operator++() { ++it; return *this; }
//         bool operator!=(const Iterator& other) const { return it != other.it; }
//         auto& operator*() const { return *it; }
//     };

//     // Begin and end methods for range-based for loops
//     Iterator begin() {
//       if constexpr (Start >= 0){
//         return Iterator(Fact::Iterator(&*fact->begin()  + Start) ); 
//       }else{
//         return Iterator(Fact::Iterator(&*fact->end() + Start) ); 
//       } 
//     }
//     Iterator end() { 
//       if constexpr (End >= 0){
//         return Iterator(Fact::Iterator(&*fact->begin() + End)); 
//       }else{
//         return Iterator(Fact::Iterator(&*fact->end() + End));
//       }
//     }

//     // Subscript operator for direct access
//     Item operator[](size_t index) {
//       if constexpr (Start >= 0){
//         int end = End >= 0 ? End : fact->length + End;
//         if (index < 0 or index >= end - Start) {
//             throw std::out_of_range("Index out of slice range");
//         }
//         return *fact->get(Start + index);
//       }else{
//         int start = fact->length + Start;
//         int end = End >= 0 ? End : fact->length + End;
//         if (index < 0 or index >= end - start) {
//             throw std::out_of_range("Index out of slice range");
//         }
//         return *fact->get(start + index);
//       }        
//     }

//     // Size of the slice
//     size_t size() const { return _get_size(fact->length); }
//     //   if constexpr ((Start >= 0) ^ (End >= 0)){
//     //     return End - Start;   
//     //   }else{
//     //     int start = Start >= 0 ? Start : fact->length + End;
//     //     int end = End >= 0 ? End : fact->length + End;
//     //     return end-start;
//     //   }
//     // }
// };


#define SIZEOF_FACT(n) {sizeof(Fact)+n*sizeof(Item)}


#endif /* _CRE_FACT_H_ */
