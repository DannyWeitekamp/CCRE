#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include <cstdint>
#include <iostream>
#include <string_view>
#include "../include/t_ids.h"
#include "../include/ref.h"


using std::cout;
using std::endl;
// Forward Declarations
struct Fact;
struct Var;
struct Func;
// struct UnicodeItem;


struct UnicodeItem {
    const char* data;
    // uint64_t hash;
    uint16_t t_id;
    uint8_t is_ref;
    uint8_t borrows;

    // TODO: Need a way of encoding larger bytewidths
    // uint8_t kind;
    // uint8_t is_ascii;
    uint32_t length;
};

struct ObjItem {
    void* data;
    // uint64_t hash;
    uint16_t t_id;
    uint8_t is_ref;
    uint8_t borrows;
    uint16_t pad[2];
};

struct Item {
    uint64_t val;
    // uint64_t hash;
    uint16_t t_id;
    uint8_t is_ref;
    uint8_t borrows;
    uint16_t pad[2];

    bool operator==(const Item& other) const;

    Item() : val(0), t_id(0),
             is_ref(0), borrows(0), pad(0) 
    {};

    // NOTE: Default copy constructor okay
    // Item(const Item& item) : val(item.val), t_id(item.t_id),
    //          is_ref(item.is_ref), borrows(item.borrows), pad(item.pad) 
    // {};



    Item(std::nullptr_t arg) : val(std::bit_cast<uint64_t>(arg)),
                    t_id(T_ID_NULL),
                    is_ref(0), borrows(0), pad(0)
    {};

    Item(bool arg) : val(static_cast<uint64_t>(arg)),
                    t_id(T_ID_BOOL),
                    is_ref(0), borrows(0), pad(0)
    {};

    Item(void* arg) : val(std::bit_cast<uint64_t>(arg)),
                    t_id(T_ID_NULL),
                    is_ref(0), borrows(0), pad(0)
    {};

    template <std::integral T>
    Item(const T& x) : val(x), t_id(T_ID_INT),
                        is_ref(0), borrows(0), pad(0)
    {}

    template <std::floating_point T>
    Item(const T& x) : val(std::bit_cast<uint64_t>(double(x))),
                       t_id(T_ID_FLOAT),
                       is_ref(0), borrows(0), pad(0)
    {}

    Item(const std::string_view& arg);
    Item(const char* data, size_t _length=-1);

    Item(Fact* x, uint8_t _is_ref=0xFF);
    Item(ref<Fact> x, uint8_t _is_ref=0xFF);

    Item(Var* x);
    Item(ref<Var> x);

    Item(Func* x);
    Item(ref<Func> x);

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
        UnicodeItem* ut = std::bit_cast<UnicodeItem*>(this);
        return std::string_view(ut->data, ut->length);
    }

    inline Fact* as_fact() const {
        return std::bit_cast<Fact*>(val);
    }

    inline Var* as_var() const {
        return std::bit_cast<Var*>(val);
    }

    inline Func* as_func() const {
        return std::bit_cast<Func*>(val);
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








#endif // _ITEM_H_
