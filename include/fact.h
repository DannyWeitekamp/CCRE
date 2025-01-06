#ifndef _CRE_FACT_H_
#define _CRE_FACT_H_

#include <vector>
#include <bit>
#include <iostream>
#include "../include/ref.h"
#include "../include/alloc_buffer.h"
#include "../include/item.h"
#include "../include/cre_obj.h"
#include "../include/types.h"
#include "../include/context.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/member.h"




// Externally Defined Forward Declares
struct FactSet;
struct AllocBuffer;
// extern CRE_Context* current_context;

void Fact_dtor(const CRE_Obj* x);


struct Fact : public CRE_Obj {

public: 
  //  -- Members --
  FactType* type=nullptr;
  FactSet* 	parent=nullptr; 
  uint32_t  f_id=-1;
  uint32_t  length=0;
  uint64_t 	hash=0;

  // For internal use;
  uint8_t cov_flag=0;
  // Whether or the fact is immutable;
  uint8_t immutable=0;
  uint8_t has_mutable=0;

  uint8_t pad1[5]={0};

  //  -- Methods --
  // Fact(void* type);
  Fact(FactType* _type, uint32_t _length) :
    CRE_Obj(&Fact_dtor),
    type(_type),
    parent(nullptr),
    f_id(-1),
    length(_type ? std::max(_length, uint32_t(_type->members.size())) : _length),
    hash(0),
    immutable(false)
{}


  // ~Fact(){}

  // void operator delete(void * p);
    //

  

  // Executes type specific .set()
  template<class T>
  inline void set(uint32_t ind, const T& val){
    if(ind >= length){
      throw std::out_of_range("Setting fact member [" + std::to_string(ind) + "] beyond its length (" + std::to_string(length) + ").");
    }

    // Convert to Item, always intern strings, always hash on assignment;
    Member member;
    if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>) {
      InternStr intern_str(intern(val));
      member = Member(intern_str, intern_str.hash);
    }else{
      uint64_t hash = CREHash{}(val); 
      member = Member(val, hash);
    }

    // Handle type checking against the fact's type   
    if(this->type != nullptr && ind < this->type->members.size()){
      CRE_Type* mbr_type = this->type->members[ind].type;

      if(member.t_id == T_ID_NULL){
        member.t_id = mbr_type->t_id;
      }else if(mbr_type->t_id != member.t_id && mbr_type->t_id != 0){
        CRE_Type* type = current_context->types[member.t_id-1];
        throw std::invalid_argument("Setting item[" + std::to_string(ind) + "] with type '" + mbr_type->name + "' to value " \
              + item_to_string(member) + " with type '" + type->name + "'");
      }
    }

    // Get pointer to the 0th Item
    Member* data_ptr(std::bit_cast<Member*>(
        ((uint8_t*) this) + sizeof(Fact)
    ));
    // Assign to the ind'th item
    data_ptr[ind] = member;
  }

  // inline void set_unsafe(uint32_t ind, const Item& val){
  //   set_unsafe(ind, Member(val));
  // }

  template<class T>
  inline void set_unsafe(uint32_t ind, const T& val){
    Member* data_ptr = std::bit_cast<Member*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    data_ptr[ind] = Member(val);
  }

  
  /* TODO: What is up with these?
  inline Item& operator[](uint32_t ind) {
    Item* data_ptr = std::bit_cast<Item*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return data_ptr[a_indid];
  }

  inline Item operator[](uint32_t ind) const {
    Item* data_ptr = std::bit_cast<Item*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return data_ptr[ind];
  }
  */

  inline Member get(uint32_t ind) const {
    Member* data_ptr = std::bit_cast<Member*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return data_ptr[ind];
  }

  inline Member* get_ptr(uint32_t ind) const {
    Member* data_ptr = std::bit_cast<Member*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return &(data_ptr[ind]);
  }

  inline Member get(const std::string_view& attr) const {
    if(type == nullptr){
      throw std::invalid_argument("Attribute name [\"" + std::string(attr) +
          "\"] undefined for untyped Fact.");
    }

    int index = type->get_attr_index(attr);
    if(index == -1){
      throw std::invalid_argument("No attribute [\"" + std::string(attr) +
          "\"] in Fact of type \"" + type->name + "\".");
    }

    Member* data_ptr = std::bit_cast<Member*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return data_ptr[index];
  }

  // inline Item get(uint32_t ind) const {
  //   Item* data_ptr = std::bit_cast<Item*>(
  //       std::bit_cast<uint64_t>(this) + sizeof(Fact)
  //   );
  //   return data_ptr[ind];
  // }
  
  
  // std::vector<Item*> get_items() const;
  std::vector<Member> get_members() const;
  // std::string to_string();
  inline size_t size() const {return length;}

  std::tuple<size_t, size_t> _format_slice(int _start, int _end);
  ref<Fact> slice_into(Fact* new_fact,      int start, int end, bool deep_copy=false);
  ref<Fact> slice_into(AllocBuffer& buffer, int start, int end, bool deep_copy=false);
  ref<Fact> slice(int start, int end, bool deep_copy=false);

  ref<Fact> copy_into(Fact* new_fact, bool deep_copy=false);
  ref<Fact> copy_into(AllocBuffer& buffer, bool deep_copy=false);
  ref<Fact> copy(bool deep_copy);

  bool operator==(const Fact& other) const;

  std::string get_unique_id();
  std::string to_string(uint8_t verbosity=DEFAULT_VERBOSITY);


  class Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Member;
    using pointer           = Member*;
    using reference         = Member&;

    private:
        Member* current;
    public:
        explicit Iterator(Member* item_ptr) : current(item_ptr) {}        
        reference operator*() const { return *current; }
        pointer operator->() const { return &*current; }
        Iterator& operator++() { ++current; return *this; }
        bool operator!=(const Iterator& other) const { return current != other.current; }
        bool operator==(const Iterator& other) const { return current == other.current; }
  };

  Iterator begin() const {
    uint8_t* data = (uint8_t*) this;
    return Iterator((Member*) (data + sizeof(Fact)));
  }
  Iterator end() const {
    uint8_t* data = (uint8_t*) this;
    return Iterator((Member*) (data + sizeof(Fact) + this->length*sizeof(Member)));
  }




  Member operator[](size_t index) {
    return this->get(index);
  }
};

// Enable Iteration over Fact* 
Fact::Iterator begin(const Fact* fact);
Fact::Iterator end(const Fact* fact);



std::ostream& operator<<(std::ostream& out, Fact* fact);
std::ostream& operator<<(std::ostream& out, ref<Fact> fact);



// inline void _init_fact(Fact* fact, uint32_t _length, FactType* type){
//     fact->type = type;
//     fact->f_id = 0;
//     fact->hash = 0;
//     fact->length = _length;
//     fact->parent = (FactSet*) nullptr;
// }


// inline void _init_fact(Fact* fact, uint32_t _length, FactType* _type=nullptr, bool _immutable=false){
//   // Placement new
//   new (fact) Fact(_length, _type, _immutable);
// }

ref<Fact> empty_fact(FactType* type);
ref<Fact> empty_untyped_fact(uint32_t _length);

// ref<Fact> new_fact(Fact* fact, FactType* type, const Item* items, size_t n_items);
// ref<Fact> new_fact(FactType* type, const Item* items, size_t n_items);
// ref<Fact> new_fact(Fact* fact, FactType* type, const std::vector<Item>& items);
// ref<Fact> new_fact(FactType* type, const std::vector<Item>& items);





// TODO ???
// std::string fact_to_unique_id(Fact* fact);
// std::string fact_to_string(Fact* fact, uint8_t verbosity=DEFAULT_VERBOSITY);
// extern "C" Item fact_to_item(Fact* fact);


template <class ... Ts>
ref<Fact> make_fact(FactType* type, Ts && ... inputs){
  // std::cout << std::endl;
  Member mbrs[sizeof...(Ts)];
  // std::vector<Item> items = {};
  // items.reserve();
  int i = 0;
  ([&]
    {
        // Do things in your "loop" lambda
        // Item item = to_item(inputs);
        // items.push_back(item);
        // cout << "!!" << Member(inputs) << " " << sizeof(Member) << " " << endl;
        mbrs[i] = Member(inputs);
        ++i;
        
    } (), ...);
  ref<Fact> fact = new_fact(type, mbrs, i);
  
  return fact;
}


//-------------------------------------------------------------
// : FactView

struct FactView {
    // -- Members --
    ref<Fact> fact;
    uint16_t start;
    uint16_t end_;


    // -- Methods --
    explicit FactView(Fact* _fact, int _start, int _end) :
        fact(_fact) {

        start = uint16_t(_start >= 0 ? _start : fact->length + _start);
        end_ = uint16_t(_end >= 0 ? _end : fact->length + _end);

      // #ifndef NDEBUG
        if (end_ < start ||
            start >= fact->length ||
            end_ > fact->length) {
            throw std::out_of_range("Invalid slice range[" 
              + std::to_string(start) + "," + std::to_string(end_) +
              "] for Fact with length=" + std::to_string(fact->length) + ".");
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

    Member get(size_t index) {
      if (index >= end_ - start) {
          throw std::out_of_range("Index out of slice range");
      }
      return fact->get(start + index);
    }
    

    // Subscript operator for direct access
    Item operator[](size_t index) {
      return this->get(index);
    }

    // Size of the view
    inline size_t size() const { return end_-start; }

    bool operator==(const FactView& other) const;
    
};

// ------------------------------------------------------------
// : SIZEOF_FACT(n)

constexpr bool FACT_ALIGN_IS_POW2 = (alignof(Fact) & (alignof(Fact) - 1)) == 0;
#define _SIZEOF_FACT(n) (sizeof(Fact)+(n)*sizeof(Member))

// Because facts are regularly allocated with buffers and have an atomic
//  for their refcount we need to make sure to pad facts so that
//  ((Fact*) x) + SIZEOF_FACT(x->size()) is always an aligned address so 
//  that we keep facts in contigous memory that wasn't allocated with 'new'
#if FACT_ALIGN_IS_POW2 == true
  #define ALIGN_PADDING(n_bytes) ((alignof(Fact) - (n_bytes & (alignof(Fact)-1))) & (alignof(Fact)-1))
#else
  #define ALIGN_PADDING(n_bytes) ((alignof(Fact) - (n_bytes % (alignof(Fact)))) % (alignof(Fact)))
#endif

constexpr bool FACT_NEED_ALIGN_PAD = (ALIGN_PADDING(sizeof(Fact)) | ALIGN_PADDING(sizeof(Member))) != 0;

#if FACT_NEED_ALIGN_PAD
  #define SIZEOF_FACT(n) (_SIZEOF_FACT(n) + ALIGN_PADDING(_SIZEOF_FACT(n)))
#else
  #define SIZEOF_FACT(n) _SIZEOF_FACT(n)
#endif


// ---------------------
// : new_fact() + alloc_fact() + empty_fact()

inline uint32_t _resolve_fact_len(FactType* type, size_t n_items){
  // Resolve length + finalize type
  uint32_t length;
  if(type == nullptr){
    length = uint32_t(n_items);
  }else{
    type->ensure_finalized();
    length = std::max(uint32_t(type->members.size()), uint32_t(n_items));
  }
  return length;
}


inline ref<Fact> alloc_fact(FactType* type, uint32_t length=0){
  length = _resolve_fact_len(type, length); 
  Fact* fact_addr = (Fact*) malloc(SIZEOF_FACT(length));

  ref<Fact> fact = new (fact_addr) Fact(type, length);

  return fact;
}

inline void _zfill_fact(Fact* fact, uint32_t start, uint32_t end){
  // Pointer to the 0th Item of the FactHeader
  Member* data_ptr = std::bit_cast<Member*>(
      std::bit_cast<uint64_t>(fact) + sizeof(Fact)
  );
  FactType* type = fact->type;
  // cout << "ZFILL: " << start << ", " << end << endl;
  for(int ind = start; ind < end; ind++){
      data_ptr[ind] = Member();

      if(type != nullptr && ind < type->members.size()){
        CRE_Type* mbr_typ = (&type->members[ind])->type;
        // cout << uint64_t(mbr_typ) << " FILL BACK" << std::to_string(ind) <<
        //      " T_ID: " << mbr_typ->t_id <<
        //      " Type: " << mbr_typ << " [" << 
        //                int(mbr_typ->kind) << "]" << endl;
        data_ptr[ind].t_id = mbr_typ->t_id;
      }
  }
}



template<std::derived_from<Item> ItemOrMbr>
inline void _fill_fact(Fact* fact, const ItemOrMbr* items, size_t n_items){
  // Set items
  try{
    for(int i=0; i < n_items; i++){

      // Note/Verify TODO: copy elision optimization should prevent copying
      //   this temporary.
      Item item = items[i];

      fact->set(i, item);

      if(!item.is_primitive() && item.borrows){
        ((CRE_Obj*) item.val)->inc_ref();
      }
    }
  } catch (const std::exception& e) {
    // Make sure that fact is freed on error
    Fact_dtor(fact);
    throw;
  }
  
  // Zero-fill any trailing items
  _zfill_fact(fact, n_items, fact->length);
}

inline ref<Fact> empty_fact(FactType* type,
                            uint32_t length=0){
    ref<Fact> fact = alloc_fact(type, length);
    _zfill_fact(fact, 0, fact->length);
    return fact;
}


template<std::derived_from<Item> ItemOrMbr>
ref<Fact> new_fact(FactType* type, const ItemOrMbr* items, size_t n_items, bool immutable=false){
  ref<Fact> fact = alloc_fact(type, n_items);
  _fill_fact(fact, items, n_items);
  return fact;
}

template<std::derived_from<Item> ItemOrMbr>
ref<Fact> new_fact(FactType* type, const std::vector<ItemOrMbr>& items, bool immutable=false){
  return new_fact(type, items.data(), items.size());
}


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



#endif /* _CRE_FACT_H_ */
