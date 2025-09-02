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
class Fact;
struct Var;
struct Func;
class CRE_Obj;
struct ControlBlock;

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
    // Either a ctrl_block pointer or other data
private:
    // union{
        // ControlBlock* ctrl_block;

    //[Bytes 8-12]
    // Length or padding (type dependant)
    union {
        uint32_t length;
        uint16_t pad[2];
    };
    // [Bytes 12-14]
    // The type identifier of the Item
    uint16_t t_id;

    // [Bytes 14-15]
    // Extra info (type dependant)
    uint8_t meta_data;

    // [Bytes 15-16]
    // Determines if value-like or a kind of reference 
    uint8_t val_kind;
    // };

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

    // inline ControlBlock* get_ctrl_block() const {
    //     return (ControlBlock*) ((uint64_t(ctrl_block)>>3)<<3);
    // }


    inline bool is_expired() const {
        if(is_wref()){
            ControlBlock* cb = (ControlBlock*) ptr;
            // cout << "is_expired: " << uint64_t(cb) << endl;
            return cb->is_expired();
        }
        return false;
    }

    inline uint16_t get_t_id() const {
        return t_id;   
    }
        // if(is_wref()){
        //     // cout << "get_t_id" << endl;
        //     ControlBlock* cb = (ControlBlock*) ptr;
        //     if(cb->is_expired()){
        //         return T_ID_UNDEF;
        //     }
        //     // cout << 

        //     // return t_id;
        //     // ControlBlock* cb = (ControlBlock*) ptr;//get_ctrl_block();
        //     // // cout << "T_ID FROM CNTRL BLOCK:" << ctrl_block->t_id << endl;
        //     // return cb->t_id;
        // }
        

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
        if(this->get_t_id() == other_item.get_t_id()){
            if(!is_value()){
                return this->get_ptr() == other_item.get_ptr();
            }else{
                return this->val == other_item.val;
            }
        }
        return false;
    }

    


    Item() : val(0), t_id(T_ID_UNDEF),
             meta_data(0), val_kind(VALUE), pad(0) 
    {};

    // void _force_strong() {
    //     if(is_wref() && !is_expired()){
    //         ControlBlock* cb = (ControlBlock*) ptr;
    //         ptr = (CRE_Obj*) cb->obj_ptr;
    //         val_kind = STRONG_REF;
    //     }
    //     borrow();
    // }

  

    Item(const Item& other) :
         val(other.val), t_id(other.t_id),
         meta_data(other.meta_data), val_kind(other.val_kind), length(other.length) 
    {
        // Force copies to be strong refs
        // _force_strong();
        borrow();
    };

    Item& operator=(Item&& other) {    
        release();// Note: Valgrind Doesn't like this 
        val = other.val;
        t_id = other.t_id;
        meta_data = other.meta_data;
        val_kind = other.val_kind;
        length = other.length;

        // Release from other
        other.ptr = nullptr;
        other.t_id = T_ID_UNDEF;
        other.val_kind = VALUE;
        return *this;
    };

    Item& operator=(const Item& other) {    
        other.borrow();
        release();
        val = other.val;
        t_id = other.t_id;
        meta_data = other.meta_data;
        val_kind = other.val_kind;
        length = other.length;
        return *this;
    };

    

    Item(const Item& other, uint8_t val_kind) :
         // ptr(other.ptr),
          t_id(other.t_id),
         meta_data(other.meta_data), val_kind(val_kind), length(other.length) 
    {
        CRE_Obj* other_ptr = other.get_ptr();
        if(val_kind == WEAK_REF && other_ptr != nullptr){
            ptr = other_ptr->control_block;
        }else{
            ptr = other_ptr;
        }
        // cout << "BEFORE BORROW" << other.get_wrefcount() << ", " << this->get_wrefcount() << endl;
        borrow();
    };

    

    // Item& operator=(Item&&) = default;
    Item(Item&&) = default;

    // Item(uint64_t _val, uint16_t _t_id, uint32_t length,  uint8_t meta_data, uint8_t)

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
                    meta_data(0), val_kind(RAW_PTR), pad(0)
    {};


    
    // explicit Item(Fact* x) : 
    //   ptr((void *) x),
    //   t_id(T_ID_FACT),
    //   meta_data(0), 
    //   val_kind(RAW_PTR),
    //   pad(0)           
    // {
    //     // if(x != nullptr){
    //     //     borrow();
    //     // }
    // }

    template <class T>
    explicit Item(T* x) : 
      ptr((void *) x),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(RAW_PTR),
      pad(0)           
    {}

    template <class T>
    explicit Item(const ref<T>& x) : 
      ptr((void *) x.get()),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(STRONG_REF),
      pad(0)
    {
        x->inc_ref(); 
    }

    template <class T>
    explicit Item(const wref<T>& x) : 
      ptr((void *) x.get_cb()),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(WEAK_REF),
      pad(0)
      // meta_data(0), 
      // val_kind(WEAK_REF),
      // ctrl_block((ControlBlock*) (uint64_t(x.get_cb()) | WEAK_REF) )
    {
        // cout << "ITEM CTRL BLOCK" << x.get_cb() << endl;
        x->inc_wref(); 
    }


    Item(const std::string_view& arg);
    Item(const char* data, size_t _length=-1);

    // Item(Fact* x, uint8_t _is_ref=0xFF);
    // Item(ref<Fact> x, uint8_t _is_ref=0xFF);

    // Item(Var* x);
    // Item(ref<Var> x);

    // Item(Func* x);
    // Item(ref<Func> x);

    void borrow() const;
    void release() const;

    ~Item(){
        release();
    };



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




    template <std::integral T>
    T as(){
        return T(std::bit_cast<int64_t>(val));
    }

    template <std::floating_point T>
    T as(){
        return T(std::bit_cast<double>(val));
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

    inline CRE_Obj* get_ptr() const{
        if(is_wref()){
            ControlBlock* cb = (ControlBlock*) ptr;
            if(cb->is_expired()) [[unlikely]] { 
                return nullptr;
            }
            
            return std::bit_cast<CRE_Obj*>(cb->obj_ptr);
        }
        // cout << "EENDL" << endl;
        return std::bit_cast<CRE_Obj*>(ptr);
    }

    inline ControlBlock* get_cb() const{
        if(is_wref()){
            return (ControlBlock*) ptr;
        }else{
            
            if(ptr == nullptr) return nullptr;
            return ((CRE_Obj*) ptr)->control_block;
        }
        // cout << "EENDL" << endl;
        // return std::bit_cast<CRE_Obj*>(ptr);
    }

    inline Fact* as_fact() const {
        // cout << "as fact: " << uint64_t(get_ptr()) << endl; 
        return std::bit_cast<Fact*>(get_ptr());
    }

    // Keeping this purely for benchmarking to justify the 
    //   use of the bitpacking trickery in fast method
    // inline Fact* as_fact_slow() const {
    //     ControlBlock* cb = get_ctrl_block();
    //     if(cb->is_expired()) [[unlikely]] { 
    //         return nullptr;
    //     }
    //     return std::bit_cast<Fact*>(cb->obj_ptr);
    // }

    inline Var* as_var() const {
        return std::bit_cast<Var*>(get_ptr());
    }

    inline Func* as_func() const {
        return std::bit_cast<Func*>(get_ptr());
    }

    // template <std::derived_from<T, CRE_Obj> ref<T>>
    // ref<T> as(){
    //     return std::bit_cast<T*>(get_ptr());
    // }

    template <typename T>
    requires std::is_pointer_v<T>
    T as(){
        return std::bit_cast<T>(get_ptr());
    }

    // inline void to_weak() {    
    //     Item copy = *this;
    //     if(is_ref() && !is_wref()){
    //         ControlBlock* cb = ((CRE_Obj*) ptr)->control_block;
    //         return Item(wref<ControlBlock*>(cb));
    //     }else{
    //         return Item(wref<CRE_Obj*>(ptr->obj_ptr));
    //     }

    //     copy.val_kind = WEAK_REF
    //     return 
    // }
    inline Item to_weak(){
        return Item(this, WEAK_REF);
    }

    inline Item to_strong(){
        return Item(this, STRONG_REF);        
    }

    inline Item to_raw_ptr(){
        return Item(this, RAW_PTR);
    }

    size_t get_refcount() const {
        if(is_value()){
            throw std::runtime_error("Cannot get refcount Item does not represent CRE_Obj.");
        }
        CRE_Obj* obj = get_ptr();
        if(!obj){
            return 0;
        }
        return obj->get_refcount();
    }

    size_t get_wrefcount() const {
        if(is_value()){
            throw std::runtime_error("Cannot get wrefcount Item does not represent CRE_Obj.");
        }
        ControlBlock* cb = get_cb();
        if(!cb){
            return 0;
        }
        return cb->get_wrefcount();
    }



    // inline void make_weak() {
    //     // cout << "?MAKE WEAK " << is_ref() << " " << !is_wref() << endl;
    //     if(is_ref() && !is_wref()){
    //         // cout << "MAKE WEAK:" << this->to_string() << endl;//<< uint64_t(cb) << endl;
    //         // val_kind = ((val_kind >> 3) << 3) ;
    //         ControlBlock* cb = ((CRE_Obj*) ptr)->control_block;

    //         // cout << "MAKE WEAK:" << uint64_t(cb) << endl;
    //         cb->inc_wref();
    //         if(is_ref()){
    //             release();
    //         }

    //         ptr = (void*) cb;
    //         val_kind = WEAK_REF;

    //         // ctrl_block = (ControlBlock*) (uint64_t(cb) | WEAK_REF);


    //         // cout << "IS WEAK aft:" << is_wref() << ", " << (uint64_t(ctrl_block) & 3) << ", " <<
    //             // int(val_kind) << ", " << (val_kind & 3) << ", " << (uint64_t(ctrl_block) & 3) << endl;
    //         // cout << "AS WEAK:" << this->to_string() << endl;//<< uint64_t(cb) << endl;
    //     }
    // }

    // inline void make_strong() {
    //     if(is_ref() && !is_sref()){
    //         // cout << "MAKE WEAK:" << this->to_string() << endl;//<< uint64_t(cb) << endl;
    //         // val_kind = ((val_kind >> 3) << 3) ;
    //         ControlBlock* cb = (ControlBlock*) ptr;
    //         CRE_Obj* obj_ptr = cb->obj_ptr;

    //         // cout << "MAKE WEAK:" << uint64_t(cb) << endl;
    //         obj_ptr->inc_ref();
    //         if(is_ref()){
    //             release();
    //         }

    //         ptr = (void*) cb->obj_ptr;
    //         val_kind = STRONG_REF;

    //         // ctrl_block = (ControlBlock*) (uint64_t(cb) | WEAK_REF);


    //         // cout << "IS WEAK aft:" << is_wref() << ", " << (uint64_t(ctrl_block) & 3) << ", " <<
    //             // int(val_kind) << ", " << (val_kind & 3) << ", " << (uint64_t(ctrl_block) & 3) << endl;
    //         // cout << "AS WEAK:" << this->to_string() << endl;//<< uint64_t(cb) << endl;
    //     }
    // }

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
