#pragma once

#include <vector>
#include <bit>
#include <iostream>
#include "../include/ref.h"
#include "../include/wref.h"
#include "../include/alloc_buffer.h"
#include "../include/item.h"
#include "../include/cre_obj.h"
#include "../include/types.h"
#include "../include/context.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/member.h"

namespace cre {

const uint8_t COPY_SHALLOW = 0;
const uint8_t COPY_DEEP = 1;
const uint8_t COPY_DEEP_REFS = 2;


// Externally Defined Forward Declares
struct FactSet;
struct AllocBuffer;
// extern CRE_Context* current_context;

void Fact_dtor(const CRE_Obj* x);


struct Fact : public CRE_Obj {

public: 
  static constexpr uint16_t T_ID = T_ID_FACT;

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

  int16_t unique_id_index=-1;
  uint8_t pad1[3]={0};


  

  //  -- Methods --
  // Fact(void* type);
  Fact(FactType* _type, uint32_t _length) :   
    type(_type),
    parent(nullptr),
    f_id(-1),
    length(_type ? std::max(_length, uint32_t(_type->members.size())) : _length),
    hash(FNV_BASIS ^ (_length * FNV_PRIME)),
    immutable(false)
{
  this->init_control_block(&Fact_dtor, T_ID);
}

  void ensure_unique_id();


  // ~Fact(){}

  // void operator delete(void * p);
    //
  template<class T>
  inline Member borrow_val_as_member(const T& val){
    // Convert to Member, always intern strings, always hash on assignment;
    // cout << "L81" << endl;
    Member member;
    if constexpr (std::is_same_v<T, Member>) {
      member = val;

    }else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>) {
      InternStr intern_str(intern(val));
      member = Member(intern_str, intern_str.hash);

    }else{
      uint64_t hash = CREHash{}(val); 
      // if constexpr (std::is_base_of_v<T, Fact>) {  
      //   // Fact reference members are always weak
      //   member = Member(wref<Fact>(val), hash);
      // }else{
      member = Member(val, hash);
      // }
    }
    // cout << "L99" << endl;


    
    if(member.is_sref() || member.is_raw_ptr()){
      cout << "MEMBER: " << member.to_string() << endl; //<< "  VAL: " << val << endl; 
      cout << "MM: R: " << member.get_refcount() << " W:" << member.get_wrefcount() << "VK: " << member.is_wref() << endl;
      // cout << "BEFORE" << endl;
      member = member.to_weak();// make_weak();  

    }
    // cout << "  -  " << endl;
    member.borrow();
    return member;
  }

  inline void verify_member_type(uint32_t ind, const Item& item){
    if(this->type != nullptr && ind < this->type->members.size()){
      CRE_Type* mbr_type = this->type->members[ind].type;

      // if(member.get_t_id() == T_ID_NULL){
      //   member.get_t_id() = mbr_type->t_id;
      // }
      if(item.get_t_id() != T_ID_UNDEF && // Members can always be Undef
         item.get_t_id() != T_ID_NONE && // Members can always be None
         mbr_type->t_id != item.get_t_id() &&
         mbr_type->t_id != 0){

        CRE_Type* type = current_context->types[item.get_t_id()-1];
        throw std::invalid_argument("Setting item[" + std::to_string(ind) + "] with type '" + mbr_type->name + "' to value " \
          + item_to_string(item) + " with type '" + type->name + "'");
      }
      // return true;
    }
  }

  #define MBR_HASH(_mbr_hash, __ind) ( (_mbr_hash * (__ind+1) * FNV_PRIME) )

  // Executes type specific .set()
  template<class T>
  inline void set(uint32_t ind, const T& val){
    if(ind >= length){
      throw std::out_of_range("Setting fact member [" + std::to_string(ind) + "] beyond its length (" + std::to_string(length) + ").");
    }
    Member member = borrow_val_as_member(val);

    // Handle type checking against the fact's type   
    verify_member_type(ind, member);

    // Get pointer to the 0th Item
    Member* data_ptr(std::bit_cast<Member*>(
        ((uint8_t*) this) + sizeof(Fact)
    ));

    // Replace the XOR contribution of the old hash with the new
    hash ^= MBR_HASH(data_ptr[ind].hash, ind);
    hash ^= MBR_HASH(member.hash, ind);

    // Assign to the ind'th item
    // member.borrow();
    data_ptr[ind] = member;
  }

  // inline void set_unsafe(uint32_t ind, const Item& val){
  //   set_unsafe(ind, Member(val));
  // }

  /**
   * @brief A variant of `Fact::set`, for efficiently filling fact members 
   *  immediately after instantiating them with `alloc_fact`.
   *  `set_unsafe` is useful for optimizing code that will fill every 
   *  member slot in a new allocated fact, and has undefined behavior outside
   *  of this context. If you're unsure use `empty_fact` and `Fact::set` instead.
   *  
   * 
   * @param ind The member's index 
   * @param val The member's value
   * @return void 
   */
  template<class T>
  inline void set_unsafe(uint32_t ind, const T& val){
    Member member = borrow_val_as_member(val);

    Member* data_ptr = std::bit_cast<Member*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    
    // Add the XOR contribution of the member
    hash ^= MBR_HASH(member.hash, ind);

    // Emplacement new with Undef member to make valgrind happy
    new (data_ptr + ind) Member();
    // Assign to the ind'th item
    data_ptr[ind] = member;
  }

  template<class T>
  inline void set(const std::string_view& attr, const T& val) {
    if(type == nullptr){
      throw std::invalid_argument("Attribute name [\"" + std::string(attr) +
          "\"] undefined for untyped Fact.");
    }

    int index = type->get_attr_index(attr);
    if(index == -1){
      throw std::invalid_argument("No attribute [\"" + std::string(attr) +
          "\"] in Fact of type \"" + type->name + "\".");
    }

    set(index, val);

    // Member* data_ptr = std::bit_cast<Member*>(
    //     std::bit_cast<uint64_t>(this) + sizeof(Fact)
    // );
    // return data_ptr[index];
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

  inline const Member& get(uint32_t ind) const {
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

  inline const Member& get(const std::string_view& attr) const {
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

  // inline Member get(const std::string_view& attr) const {
  //   return *get(attr);
  // }

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
  // ref<Fact> slice_into(Fact* new_fact,      int start, int end, bool deep_copy=false);
  // ref<Fact> slice_into(AllocBuffer& buffer, int start, int end, bool deep_copy=false);
  ref<Fact> slice(int start, int end, uint8_t copy_kind=COPY_SHALLOW, 
                  AllocBuffer* buffer=nullptr);

  // ref<Fact> copy_into(Fact* new_fact, bool deep_copy=false);
  // ref<Fact> copy_into(AllocBuffer& buffer, bool deep_copy=false);
  ref<Fact> copy(uint8_t copy_kind=COPY_SHALLOW, 
                 AllocBuffer* buffer=nullptr);

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

// ref<Fact> empty_fact(FactType* type);
// ref<Fact> empty_untyped_fact(uint32_t _length);

// ref<Fact> new_fact(Fact* fact, FactType* type, const Item* items, size_t n_items);
// ref<Fact> new_fact(FactType* type, const Item* items, size_t n_items);
// ref<Fact> new_fact(Fact* fact, FactType* type, const std::vector<Item>& items);
// ref<Fact> new_fact(FactType* type, const std::vector<Item>& items);





// TODO ???
// std::string fact_to_unique_id(Fact* fact);
// std::string fact_to_string(Fact* fact, uint8_t verbosity=DEFAULT_VERBOSITY);
// extern "C" Item fact_to_item(Fact* fact);


template <class ... Ts>
ref<Fact> make_fact(FactType* type, const Ts& ... inputs){
  cout << "--START MAKE FACT--" << endl;

  Member mbrs[sizeof...(Ts)] = {Member()};
  int i = 0;
  ([&]
    {
        // if constexpr(std::is_base_of_v<Ts, ref<Fact>> || std::is_base_of_v<Ts, wref<Fact>>){
          // mbrs[i] = Member(inputs.get());
        // }else{
        // cout << "-START Member" << endl;

        // Construct the Member directly in mbrs[i]
        new (mbrs + i) Member(inputs);
        // mbrs[i] = std::move();
        // cout << "-" << endl;
        // mbrs[i].borrow();
        // mbrs[i].borrow();
        // }
        // cout << "-END Member" << endl;


        // if(mbrs[i].is_ref()){
        //   cout << "!! " << inputs << " r:" << mbrs[i].get_refcount() << " w:" << mbrs[i].get_wrefcount()  << endl;  
        // }
        
        
        ++i;
        
    } (), ...);

  // cout << "-START New FACT" << endl;
  ref<Fact> fact = new_fact(type, mbrs, i, false);
  // cout << "-END New FACT" << endl;

  // cout << "--END MAKE FACT--" << endl;
  
  return fact;
}

template <class ... Ts>
ref<Fact> make_tuple(const Ts& ... inputs){
  Member mbrs[sizeof...(Ts)] = {Member()};
  int i = 0;
  ([&]
    {
        // cout << "!!" << Member(inputs) << " " << sizeof(Member) << " " << endl;
      // cout << "!!" << inputs << " " << endl;
        new (mbrs + i) Member(inputs);
        // mbrs[i] = Member(inputs);
        ++i;
        
    } (), ...);
  ref<Fact> fact = new_fact(nullptr, mbrs, i, true);
  
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

const uint64_t FACT_ALIGN = alignof(Fact);
// const uint64_t FACT_ALIGN = 64;


constexpr bool FACT_ALIGN_IS_POW2 = (FACT_ALIGN & (FACT_ALIGN - 1)) == 0;
#define _SIZEOF_FACT(n) (sizeof(Fact)+(n)*sizeof(Member))

// Because facts are regularly allocated with buffers and have an atomic
//  for their refcount we need to make sure to pad facts so that
//  ((Fact*) x) + SIZEOF_FACT(x->size()) is always an aligned address so 
//  that we keep facts in contigous memory that wasn't allocated with 'new'
#if FACT_ALIGN_IS_POW2 == true
  #define ALIGN_PADDING(n_bytes) ((FACT_ALIGN - (n_bytes & (FACT_ALIGN-1))) & (FACT_ALIGN-1))
#else
  #define ALIGN_PADDING(n_bytes) ((FACT_ALIGN - (n_bytes % (FACT_ALIGN))) % (FACT_ALIGN))
#endif

constexpr bool FACT_NEED_ALIGN_PAD = ((_SIZEOF_FACT(0) % FACT_ALIGN) | (sizeof(Member) % FACT_ALIGN)) != 0;

#if FACT_NEED_ALIGN_PAD
#define SIZEOF_FACT(n) (_SIZEOF_FACT(n) + ALIGN_PADDING(_SIZEOF_FACT(n)))
#else
#define SIZEOF_FACT(n) _SIZEOF_FACT(n)
#endif


// ---------------------
// : new_fact() + alloc_fact() + empty_fact()


/**
   * @brief Returns the length of a fact with a particular type and 
   *   number of items. The length will be at least as long as the 
   *   number of members in the type specification, or longer if 
   *   n_items exceeds this. 
   * 
   * @param type A FactType 
   * @param n_items 
   * @return The fact's length
   */

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


/**
   * @brief An unsafe way to allocates a new fact. Slightly faster than 
   *  the safer empty_fact(), but has undefined behavior unless all of the 
   *  allocated member slots of a fact are gaurenteed to be filled shortly 
   *  after the allocation. Thus this should really only be used in internal
   *  subroutines.
   *  
   * 
   * @param fact A fact recently allocated with alloc_fact.
   * @param start The start index 
   * @param end The end index
   * @return The newly allocated fact
   */

inline ref<Fact> alloc_fact(FactType* type, uint32_t length=0, AllocBuffer* buffer=nullptr){
  length = _resolve_fact_len(type, length);

  bool did_malloc = false;
  Fact* fact_addr = nullptr;
  if(buffer != nullptr){
    fact_addr = (Fact*) buffer->alloc_bytes(SIZEOF_FACT(length), did_malloc);
    
  }else{
    did_malloc = true;
    fact_addr = (Fact*) malloc(SIZEOF_FACT(length));
  }

  ref<Fact> fact = new (fact_addr) Fact(type, length);

  if(!did_malloc){
      fact->control_block->alloc_buffer = buffer;
      buffer->inc_ref();
  }

  // cout << "MOO: " << uint64_t(fact.get()) << endl;

  return fact;
}


/**
   * @brief Fills a span within a newly allocated fact with NULL. 
   * 
   * @param fact A fact recently allocated with alloc_fact.
   * @param start The start index 
   * @param end The end index
   * @return void 
   */
inline void _zfill_fact(Fact* fact, uint32_t start, uint32_t end){
  // Pointer to the 0th Item of the FactHeader
  Member* data_ptr = std::bit_cast<Member*>(
      std::bit_cast<uint64_t>(fact) + sizeof(Fact)
  );
  FactType* type = fact->type;
  // cout << "ZFILL: " << start << ", " << end << endl;
  for(int ind = start; ind < end; ind++){
      // Fill with Undef Member using emplacement new 
      //  instead of assignment (makes valgrind happy)
      new (data_ptr +ind) Member();
      // cout << data_ptr[ind].get_t_id() << endl;

      // NOTE: Code for filling in t_ids of fresh (but is unecessary)
      // if(type != nullptr && ind < type->members.size()){
      //   CRE_Type* mbr_typ = (&type->members[ind])->type;
        // cout << uint64_t(mbr_typ) << " FILL BACK" << std::to_string(ind) <<
        //      " T_ID: " << mbr_typ->t_id <<
        //      " Type: " << mbr_typ << " [" << 
        //                int(mbr_typ->kind) << "]" << endl;
        // data_ptr[ind].set_t_id(mbr_typ->t_id);
      // }
  }
}


/**
   * @brief Fills a newly allocated fact with items from an array. Will
   *  fill any trailing members with NULL. Tightly coupled with alloc_fact
   *  and should be mostly reserved for internal use.
   * 
   * @param fact A fact recently allocated with alloc_fact.
   * @param items A pointer to the array of items. 
   * @param n_items The number of items in the array.
   * @return void 
   */
template<std::derived_from<Item> ItemOrMbr>
inline void _fill_fact(Fact* fact, const ItemOrMbr* items, size_t n_items){
  // Set items
  try{
    for(int i=0; i < n_items; i++){

      // Note/Verify TODO: copy elision optimization should prevent copying
      //   this temporary.
      const ItemOrMbr& item = items[i];

      
      fact->verify_member_type(i, item);

      // cout << "i=" << i << " t_id=" << item.get_t_id(); 
      // cout << "item=" << item << endl; 

      fact->set_unsafe(i, item);



      // if(!item.is_primitive() && item.borrows){
      //   ((CRE_Obj*) item.val)->inc_ref();
      // }
    }
  } catch (const std::exception& e) {
    // Make sure that fact is freed on error
    Fact_dtor(fact);
    throw;
  }
  
  // Zero-fill any trailing items
  _zfill_fact(fact, n_items, fact->length);

  // fact->hash = CREHash{}(fact);
}


/**
   * @brief Makes an empty fact, filled with NULLs. 
   * 
   * @param type The type of the fact, use nullptr for untyped facts 
   * @param length The length of the fact. If shorter than the type length,
   *   will be overrided by the type length. If longer, the fact will have,
   *   additional trailing members (which is unusual but not forbidden).
   * @return The new empty fact
   */
inline ref<Fact> empty_fact(FactType* type, uint32_t length=0,
                            AllocBuffer* buffer=nullptr){
    ref<Fact> fact = alloc_fact(type, length);
    _zfill_fact(fact, 0, fact->length);
    fact->hash = CREHash{}(fact);
    return fact;
}


// NOTE
// Facts have their hashes computed at instantiation since:
  //  1) Always having .hash hold a valid value makes rehashing
  //     cheap since the hash function is commutative by XORing members.
  //  2) At instantiation members are already in the L1-cache so it 
  //     is arguably cheaper to do here than later. And the hash function
  //     itself is a FNV-1a variant (very fast), so cache-delays would likely
  //     overpower any advantages to lazy execution.
  //  3) There are potential speculative execution benfits. One could, 
  //     in principle, remove branches where hash==0, triggering a full
  //     hash recompute. (Too early as of Jan 7, 2025 to bother with such
  //     an optimization).

template<std::derived_from<Item> ItemOrMbr>
ref<Fact> new_fact(FactType* type, const ItemOrMbr* items, size_t n_items,
                    bool immutable=false, AllocBuffer* buffer=nullptr){
  ref<Fact> fact = alloc_fact(type, n_items, buffer);
  _fill_fact(fact, items, n_items);
  fact->immutable = immutable;
  fact->ensure_unique_id();
  return fact;
}

template<std::derived_from<Item> ItemOrMbr>
ref<Fact> new_fact(FactType* type, const std::vector<ItemOrMbr>& items,
                    bool immutable=false, AllocBuffer* buffer=nullptr){
  return new_fact(type, items.data(), items.size(), buffer);
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

} // NAMESPACE_END(cre)

