#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <string_view>
#include "../include/cre_obj.h"
#include "../include/t_ids.h"
#include "../include/ref.h"
#include "../include/wref.h"

namespace cre {

using std::cout;
using std::endl;
// Forward Declarations
struct Fact;
struct Var;
struct Func;

// enum class ValueKind : uint8_t {
const uint8_t VALUE =      uint8_t(0);
const uint8_t WEAK_REF =   uint8_t(1);
const uint8_t RAW_PTR =    uint8_t(2);
const uint8_t STRONG_REF = uint8_t(3);
// };


// struct UnicodeItem;


// struct UnicodeItem {
//     const char* data;
//     // uint64_t hash;
//     uint16_t t_id;
//     uint8_t is_ref;
//     uint8_t borrows;

//     // TODO: Need a way of encoding larger bytewidths
//     // uint8_t kind;
//     // uint8_t is_ascii;
//     uint32_t length;
// };

// struct ObjItem {
//     void* data;
//     // uint64_t hash;
//     uint16_t t_id;
//     uint8_t is_ref;
//     uint8_t borrows;
//     uint16_t pad[2];
// };

struct Item{
    // [Bytes 0-8]
    // The main value or pointer of the Item
public:
    union {
        const char* data;
        void* ptr;
        uint64_t val;
    };

    // [Bytes 8-16]
    // Either a control_block pointer or other data
private:
    union{

        ControlBlock* control_block;
        struct {
            //[Bytes 8-12]
            // Length or padding (type dependant)
            union {
                uint32_t length;
                uint16_t pad[2];
            };

        
            // [Bytes 8-10]
            // The type identifier of the Item
            uint16_t t_id;

            // [Bytes 10-11]
            // Extra info (type dependant)
            uint8_t meta_data; 

            // [Bytes 11-12]
            // Determines if is value-like or pointer like 
            // NOTE: This must be last since it is packed
            //   into the control_block pointer for weak 
            //   references 
            uint8_t val_kind;
        };
    };
// }

public:


// struct Item {
//     uint64_t val;
//     // uint64_t hash;
//     uint16_t t_id;
//     uint8_t is_ref;
//     uint8_t borrows;
//     uint16_t pad[2];

    bool operator==(const Item& other) const;


    inline bool is_ref() const {
        return ptr != nullptr && bool(val_kind & 1);
    }

    inline bool is_value() const {
        return (val_kind & 3) == VALUE;
    }

    inline bool is_raw_ptr() const {
        return (val_kind & 3) == RAW_PTR;
    }

    inline bool is_sref() const {
        return ptr != nullptr && (val_kind & 3) == STRONG_REF;
    }

    inline bool is_wref() const {
        return ptr != nullptr && (val_kind & 3) == WEAK_REF;   
    }



    inline ControlBlock* get_control_block() const {
        return (ControlBlock*) ((uint64_t(control_block)>>3)<<3);
    }


    inline bool is_expired() const {
        if(is_wref()){
            ControlBlock* ctrl_block = get_control_block();
            // cout << "ctrl_block: " << int64_t(ctrl_block) << ", " << int64_t(control_block) << endl;
            return ctrl_block->is_expired();
        }
        return false;
    }

    inline uint16_t get_t_id() const {
        if(is_ref()){
            
            // TODO: Should get from block or Obj
            if(is_wref() && is_expired()){
                return T_ID_UNDEF;            
            }else{
                return T_ID_FACT;
            }
            
        }else{
            return t_id;   
        }
    }

    // inline uint16_t set_t_id(uint16_t _t_id) const {
    //     if(is_ref()){
    //         throw std::runtime_error("Cannot assign t_id to reference type.");
    //     }
    //     t_id = _t_id;
    // }

    inline bool is_undef() const {
        return get_t_id() == T_ID_UNDEF;
    }

    inline uint32_t get_length() const {
        return length;
    }


    template<class T>
    bool operator==(const T& other) const {
        Item other_item = Item(other);
        return (
            this->val == other_item.val && 
            this->get_t_id() == other_item.get_t_id()
        );
    }

    


    Item() : val(0), t_id(T_ID_UNDEF),
             meta_data(0), val_kind(VALUE), pad(0) 
    {};

    // NOTE: Default copy constructor okay
    // Item(const Item& item) : val(item.val), t_id(item.t_id),
    //          is_ref(item.is_ref), borrows(item.borrows), pad(item.pad) 
    // {};


    Item(bool arg) : val(static_cast<uint64_t>(arg)),
                    t_id(T_ID_BOOL),
                    meta_data(0), val_kind(VALUE), pad(0)
    {};



    template <std::integral T>
    Item(const T& x) : val(x), t_id(T_ID_INT),
                        meta_data(0), val_kind(VALUE), pad(0)
    {}

    template <std::floating_point T>
    Item(const T& x) : val(std::bit_cast<uint64_t>(double(x))),
                       t_id(T_ID_FLOAT),
                       meta_data(0), val_kind(VALUE), pad(0)
    {}

    Item(std::nullptr_t arg) : val(std::bit_cast<uint64_t>(arg)),
                    t_id(T_ID_UNDEF),
                    meta_data(0), val_kind(VALUE), pad(0)
    {};

    Item(void* arg) : val(std::bit_cast<uint64_t>(arg)),
                    t_id(arg == nullptr ? T_ID_UNDEF : T_ID_PTR),
                    meta_data(0), val_kind(VALUE), pad(0)
    {};

    
    explicit Item(Fact* x) : 
      ptr((void *) x),
      t_id(x == nullptr ? T_ID_UNDEF : T_ID_UNDEF),
      meta_data(0), 
      val_kind(STRONG_REF),
      pad(0)           
    {
        if(x != nullptr){
            borrow();
        }
    }

    template <class T>
    explicit Item(T* x) : 
      ptr((void *) x),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(RAW_PTR),
      pad(0)           
    {}

    template <class T>
    explicit Item(ref<T> x) : 
      ptr((void *) x),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(STRONG_REF),
      pad(0)
    {
        x->inc_ref(); 
    }

    template <class T>
    explicit Item(wref<T> x) : 
      ptr((void *) x),
      t_id(T::T_ID),
      // meta_data(0), 
      // val_kind(WEAK_REF),
      control_block((ControlBlock*) (uint64_t(x.get_cb()) | WEAK_REF) )
    {
        // cout << "ITEM CTRL BLOCK" << x.get_cb() << endl;
        x->inc_ref(); 
    }


    Item(const std::string_view& arg);
    Item(const char* data, size_t _length=-1);

    // Item(Fact* x, uint8_t _is_ref=0xFF);
    // Item(ref<Fact> x, uint8_t _is_ref=0xFF);

    // Item(Var* x);
    // Item(ref<Var> x);

    // Item(Func* x);
    // Item(ref<Func> x);

    void borrow();
    void release();


    // ~Item();
    // inline void destroy(){
    //     if(borrows){
    //         ((CRE_Obj*) val)->dec_ref();
    //     }  
    // } 


    inline bool is_primitive() const{
        return (t_id >= T_ID_BOOL && 
                t_id <= T_ID_STR);
    }


    inline bool as_bool() const {
        return bool(val);
    }

    inline int64_t as_int() const {
        return std::bit_cast<int64_t>(val);
    }

    inline double as_float() const {
        return std::bit_cast<double>(val);
    }

    inline std::string_view as_string() const {
        // UnicodeItem* ut = std::bit_cast<UnicodeItem*>(this);
        return std::string_view(this->data, this->length);
    }

    inline Fact* as_fact() const {
        if(is_expired()){ [[unlikely]]
            return nullptr;
        }
        return std::bit_cast<Fact*>(ptr);
    }

    // Keeping this purely for benchmarking to justify the 
    //   use of the bitpacking trickery in fast method
    inline Fact* as_fact_slow() const {
        ControlBlock* cb = get_control_block();
        if(cb->is_expired()){ [[unlikely]]
            return nullptr;
        }
        return std::bit_cast<Fact*>(cb->obj_ptr);
    }

    inline Var* as_var() const {
        return std::bit_cast<Var*>(val);
    }

    inline Func* as_func() const {
        return std::bit_cast<Func*>(val);
    }

    inline void make_weak() {
        // cout << "?MAKE WEAK " << uint64_t(ptr) << endl;
        if(is_ref() && !is_wref()){
            // cout << "MAKE WEAK:" << endl;//<< uint64_t(cb) << endl;
            // val_kind = ((val_kind >> 3) << 3) ;
            ControlBlock* cb = ((CRE_Obj*) ptr)->control_block;

            // cout << "MAKE WEAK:" << uint64_t(cb) << endl;
            cb->inc_wref();
            if(is_ref()){
                release();
            }

            control_block = (ControlBlock*) (uint64_t(cb) | WEAK_REF);;
        }
    }

    inline void make_strong() {
        val_kind = ((val_kind >> 3) << 3) | STRONG_REF;
    }

    std::string to_string() const;
        
};



// extern "C" Item empty_item();
// Item str_to_item(const std::string_view& arg);
// extern "C" Item str_to_item(const char* data, size_t length);
// extern "C" Item opaque_to_item(void* arg);
// extern "C" Item nullptr_to_item(std::nullptr_t arg);
// extern "C" Item bool_to_item(bool arg);
// extern "C" Item int_to_item(int64_t arg);
// extern "C" Item float_to_item(double arg);



// Item to_item(const char* arg, size_t length=-1);
// Item to_item(const std::string_view& arg);
// // Item to_item(const std::string& arg) ;
// Item to_item(std::nullptr_t arg);
// // Item to_item(void* arg);
// Item to_item(bool arg);
// Item to_item(int32_t arg);
// Item to_item(int64_t arg);
// Item to_item(uint32_t arg);
// Item to_item(uint64_t arg);
// Item to_item(double arg);
// Item to_item(float arg);
// Item to_item(Item arg);

// Item to_item(Fact* arg);

std::ostream& operator<<(std::ostream& out, Item item);

bool item_get_bool(Item item);
int64_t item_get_int(Item item);
double item_get_float(Item item);
std::string_view item_get_string(Item item);
Fact* item_get_fact(const Item& item);

std::string item_to_string(const Item& item);
std::string to_string(Item& item);

} // NAMESPACE_END(cre)
