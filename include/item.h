#pragma once

#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string_view>
#include <fmt/format.h>
#include "../include/helpers.h"
#include "../include/cre_obj.h"
#include "../include/t_ids.h"
#include "../include/ref.h"
#include "../include/wref.h"
// #include "../include/types.h"

namespace cre {

using std::cout;
using std::endl;
// Forward Declarations
class Fact;
struct Var;
struct Func;
class CRE_Obj;
struct ControlBlock;
struct CRE_Type;
struct InternStr;
struct NoneType;

// enum class ValueKind : uint8_t {
const uint8_t VALUE =      uint8_t(0);
const uint8_t WEAK_REF =   uint8_t(1);
const uint8_t RAW_PTR =    uint8_t(2);
const uint8_t STRONG_REF = uint8_t(3);
const uint8_t INTERNED = RAW_PTR | uint8_t(4);
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



    inline bool is_ref() const {
        return ptr != nullptr && bool(val_kind & 1);
    }

    inline bool is_value() const {
        return (val_kind & 3) == VALUE;
    }

    inline bool is_raw_ptr() const {
        return (val_kind & 3) == RAW_PTR;
    }

    inline bool is_interned() const {
        return val_kind == INTERNED;
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

    CRE_Type* get_type() const;
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

    
    bool operator==(const Item& other) const;

    template<class T>
    bool operator==(const T& _other) const {
        return operator==(Item(_other));
    }

    //     // if(t_id == T_ID_UNDEF && other_t_id == T_ID_UNDEF){
    //     //     return true;
    //     // }else if(t_id == T_ID_NONE && other_t_id == T_ID_NONE){
    //     //     return true;
    //     if(is_numerical(t_id) && is_numerical(other_t_id)){
    //         if(t_id == T_ID_FLOAT || other_t_id == T_ID_FLOAT){
    //             return as<double> == other.as<double>;
    //         }else{
    //             return this->val == other.val;
    //         }
    //     }else if(t_id == T_ID_STR && other_t_id == T_ID_STR){
    //         if(is_raw_ptr() || other.is_raw_ptr()){

    //         }else{

    //         }
    //     }else{
    //         return val == other.val;
    //     }

    //     if(this->get_t_id() == other.get_t_id()){
    //         if(!is_value()){
    //             return this->get_ptr() == other.get_ptr();
    //         }else{
    //             return this->val == other.val;
    //         }
    //     }
    //     return false;
    // }

    


    

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
        // cout << "ITEM COPY" << other.to_string() << endl;

        if(other.t_id == T_ID_STR and other.is_value() and other.data != nullptr){
            char* data_ptr = (char*) malloc(sizeof(char) * ( other.length+1 ));
            strcpy(data_ptr, other.data);
            data = data_ptr;
        }else{
            borrow();
        }
        // Force copies to be strong refs
        // _force_strong();
        
    };

    Item(Item&& other) :
         val(other.val), t_id(other.t_id),
         meta_data(other.meta_data), val_kind(other.val_kind), length(other.length) 
    {

        // cout << "ITEM MOVE" << other.to_string() << endl;
        other.ptr = nullptr;
        other.t_id = T_ID_UNDEF;
        other.val_kind = VALUE;
    };

    Item& operator=(const Item& other) {
        if(other.t_id == T_ID_STR and other.is_value() and other.data != nullptr){
            char* data_ptr = (char*) malloc(sizeof(char) * ( other.length+1 ));
            strcpy(data_ptr, other.data);
            release();
            data = data_ptr;
            // length = other.length;
        }else{
            other.borrow();
            release();
            val = other.val;
        }

        t_id = other.t_id;
        meta_data = other.meta_data;
        val_kind = other.val_kind;
        length = other.length;
        return *this;
    };

    Item& operator=(Item&& other) {
        release();
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

    

    Item() : val(0), t_id(T_ID_UNDEF),
             meta_data(0), val_kind(VALUE), pad(0) 
    {};

    Item([[maybe_unused]] const NoneType& none) :
         val(0), t_id(T_ID_NONE),
         meta_data(0), val_kind(VALUE), pad(0) 
    {};

    Item(const Item& other, uint8_t val_kind) :
         // ptr(other.ptr),
          t_id(other.t_id),
         meta_data(other.meta_data), val_kind(val_kind), length(other.length) 
    {
        // cout << "VAL KIND:" << uint64_t(val_kind) << endl;
        // cout << "OTHER:" << other.to_string() << endl;
        CRE_Obj* other_ptr = other.get_ptr();
        if(val_kind == WEAK_REF && other_ptr != nullptr){
            ptr = other_ptr->control_block;
        }else{
            ptr = other_ptr;
        }
        // cout << "BEFORE BORROW" << other.get_wrefcount() << ", " << this->get_wrefcount() << endl;
        borrow();
    };

    



    // Item(Item&&) = default;

    // Item(uint64_t _val, uint16_t _t_id, uint32_t length,  uint8_t meta_data, uint8_t)

    // NOTE: Default copy constructor okay
    // Item(const Item& item) : val(item.val), t_id(item.t_id),
    //          is_ref(item.is_ref), borrows(item.borrows), pad(item.pad) 
    // ;


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
    explicit Item(ref<T>&& x) : 
      ptr((void *) x.get()),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(STRONG_REF),
      pad(0)
    {
        // cout << "MOVE STRONG REF:" << x << endl;
        x->inc_ref(); 
        x.invalidate();
        
    }

    template <class T>
    explicit Item(const ref<T>& x) : 
      ptr((void *) x.get()),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(STRONG_REF),
      pad(0)
    {
        // cout << "COPY STRONG REF:" << x << endl;
        x->inc_ref();
    }

    template <class T>
    explicit Item(wref<T>&& x) : 
      ptr((void *) x.get_cb()),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(WEAK_REF),
      pad(0)
    {
        x->inc_wref();
        x.invalidate();
    }

    template <class T>
    explicit Item(const wref<T>& x) : 
      ptr((void *) x.get_cb()),
      t_id(T::T_ID),
      meta_data(0), 
      val_kind(WEAK_REF),
      pad(0)
    {
        x->inc_wref(); 
    }

    

    


    Item(const std::string& arg);
    Item(std::string_view arg);
    Item(const InternStr& arg);
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
        return t_id_is_primitive(t_id);
    }

    inline bool is_numerical() const{
        return t_id_is_numerical(t_id);
    }

    inline bool is_integral() const{
        return t_id_is_integral(t_id);
    }

    inline bool is_ptr() const{
        return t_id_is_ptr(t_id);
    }

    inline bool is_evaluatable() const{
        return t_id == T_ID_VAR || t_id == T_ID_FUNC || t_id == T_ID_LITERAL;
    }

    CRE_Type* eval_type() const;
	uint16_t eval_t_id() const ;



    inline CRE_Obj* get_ptr() const{
        if(is_wref()){
            ControlBlock* cb = (ControlBlock*) ptr;
            if(cb->is_expired()) [[unlikely]] { 
                return nullptr;
            }
            
            return std::bit_cast<CRE_Obj*>(cb->obj_ptr);
        }
        // cout << "EENDL: " << t_id << ", " << uint64_t(ptr) << endl;
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


    template <std::integral T>
    T _as() const noexcept{
        return T(std::bit_cast<int64_t>(val));
    }

    template <std::floating_point T>
    T _as() const noexcept{
        return T(std::bit_cast<double>(val));
    }
    
    template <typename T>
    requires std::is_same_v<T, std::string>
    std::string _as() const noexcept{
        return std::string(this->data, this->length);
    }

    template <typename T>
    requires std::is_same_v<T, std::string_view>
    std::string_view _as() const noexcept{
        return std::string_view(this->data, this->length);
    }

    template <typename T>
    requires std::is_pointer_v<T>
    T _as() const noexcept{
        return std::bit_cast<T>(get_ptr());
    }

    template <typename T>
    requires is_ref_v<T>
    T _as() const noexcept{
        using innerT = remove_ref_t<T>;
        return T( (innerT*) get_ptr());
    }

    template <std::integral T>
    T as() const{
        if(is_integral()){
            return _as<T>();
        }else if(t_id == T_ID_FLOAT){
            return T(_as<double>());
        }else{
            std::stringstream ss;
            ss << "Item cast to integer type failed for Item with type ";
            ss << get_type() << ".";
            throw std::runtime_error(ss.str());
        }        
    }

    template <std::floating_point T>
    T as() const {
        if(t_id == T_ID_FLOAT){
            return _as<T>();
        }else if(is_integral()){
            return T(_as<int64_t>());
        }else{
            std::stringstream ss;
            ss << "Item cast to float type failed for Item with type ";
            ss << get_type() << ".";
            throw std::runtime_error(ss.str());
        }
    }

    template <typename T>
    requires std::is_pointer_v<T>
    T as() const{
        if(is_ptr()){
            return _as<T>();    
        }else{
            std::stringstream ss;
            ss << "Item cast to pointer type failed for Item with type ";
            ss << get_type() << ".";
            throw std::runtime_error(ss.str());
        }
    }

    template <typename T>
    requires is_ref_v<T>
    T as() const {
        // TODO: should check that t_id is okay.
        if(is_ptr()){
            return _as<T>();    
        }else{
            std::stringstream ss;
            ss << "Item cast to pointer type failed for Item with type ";
            ss << get_type() << ".";
            throw std::runtime_error(ss.str());
        }
    }

    template <typename T>
    requires std::is_same_v<T, std::string>
    std::string as() const {
        switch(t_id){
        case T_ID_STR: 
            return std::string(this->data, this->length);
        case T_ID_UNDEF:
            return "Undef";
        case T_ID_NONE: 
            return "None";
        case T_ID_BOOL: 
            return val ? "True" : "False";
        case T_ID_INT: 
            return int_to_str(_as<int64_t>());
        case T_ID_FLOAT: 
            return flt_to_str(_as<double>());
        case T_ID_PTR:
            return fmt::format("{:#x}", uintptr_t(_as<void*>()));
        default:
            break;
        }
        std::stringstream ss;
        ss << fmt::format("{:#x}", uintptr_t(get_ptr()));
        return ss.str();
    }

    template <typename T>
    requires std::is_same_v<T, std::string_view>
    std::string_view as() const {
        if(t_id == T_ID_STR){
            return std::string_view(this->data, this->length);
        }else{
            std::stringstream ss;
            ss << "Item cast to string_view failed for Item with type ";
            ss << get_type() << ".";
            throw std::runtime_error(ss.str());
        }
    }
    

    

    // inline bool as_bool() const {
    //     return bool(val);
    // }

    // inline int64_t as_int() const {
    //     return std::bit_cast<int64_t>(val);
    // }

    // inline double as_float() const {
    //     return std::bit_cast<double>(val);
    // }

    // inline std::string_view as_string() const {
    //     // UnicodeItem* ut = std::bit_cast<UnicodeItem*>(this);
    //     return std::string_view(this->data, this->length);
    // }

    // inline Fact* as_fact() const {
    //     // cout << "as fact: " << uint64_t(get_ptr()) << endl; 
    //     return std::bit_cast<Fact*>(get_ptr());
    // }

    // Keeping this purely for benchmarking to justify the 
    //   use of the bitpacking trickery in fast method
    // inline Fact* as_fact_slow() const {
    //     ControlBlock* cb = get_ctrl_block();
    //     if(cb->is_expired()) [[unlikely]] { 
    //         return nullptr;
    //     }
    //     return std::bit_cast<Fact*>(cb->obj_ptr);
    // }

    // inline Var* as_var() const {
    //     return std::bit_cast<Var*>(get_ptr());
    // }

    // inline Func* as_func() const {
    //     return std::bit_cast<Func*>(get_ptr());
    // }

    // template <std::derived_from<T, CRE_Obj> ref<T>>
    // ref<T> as(){
    //     return std::bit_cast<T*>(get_ptr());
    // }

    

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
    inline Item to_weak() const{
        return Item(*this, WEAK_REF);
    }

    inline Item to_strong() const{
        return Item(*this, STRONG_REF);        
    }

    inline Item to_raw_ptr() const{
        return Item(*this, RAW_PTR);
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

    // Truthiness 
    explicit operator bool() const {
        if(t_id == T_ID_UNDEF || t_id == T_ID_NONE){
            return false;
        }else if(t_id == T_ID_STR){
            return length != 0;
        }else{
            return val != 0;
        }
    }

    friend struct Member;
        
};

bool items_equal(const Item& item1, const Item& item2, bool semantic=true, bool castable=false);

// // Same as Item, but gaurenteed to be interned
// struct InternItem : public Item {
//     using Item::Item;  // Inherit all constructors from Item

//     bool operator==(const InternItem& other) const;
// };





// extern "C" Item empty_item();
// Item str_to_item(std::string_view arg);
// extern "C" Item str_to_item(const char* data, size_t length);
// extern "C" Item opaque_to_item(void* arg);
// extern "C" Item nullptr_to_item(std::nullptr_t arg);
// extern "C" Item bool_to_item(bool arg);
// extern "C" Item int_to_item(int64_t arg);
// extern "C" Item float_to_item(double arg);



// Item to_item(const char* arg, size_t length=-1);
// Item to_item(std::string_view arg);
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

std::ostream& operator<<(std::ostream& out, const Item& item);

bool item_get_bool(Item item);
int64_t item_get_int(Item item);
double item_get_float(Item item);
std::string_view item_get_string(Item item);
Fact* item_get_fact(const Item& item);

std::string item_to_string(const Item& item);
std::string to_string(Item& item);

} // NAMESPACE_END(cre)
