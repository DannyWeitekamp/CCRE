#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include <cstdint>
#include <iostream>
#include <string_view>
#include "../include/t_ids.h"

// Forward Declarations
struct Fact;
// struct UnicodeItem;


struct UnicodeItem {
    const char* data;
    uint64_t hash;
    uint16_t t_id;
    uint8_t kind;
    uint8_t is_ascii;
    uint32_t length;
};

struct ObjItem {
    void* data;
    uint64_t hash;
    uint16_t t_id;
    uint16_t pad[3];
};

struct EmptyBlock{
  int64_t prev_f_id;
  int64_t next_f_id;
  uint16_t t_id;
  uint16_t is_lead;
  uint32_t length;
};

struct Item {
    uint64_t val;
    uint64_t hash;
    uint16_t t_id;
    uint16_t pad[3];

    bool operator==(const Item& other) const;

    Item() : val(0), hash(0), t_id(0), pad(0) {}
    Item(std::nullptr_t arg) : val(std::bit_cast<uint64_t>(arg)),
                    hash(0), t_id(T_ID_NULL){}

    Item(bool arg) : val(static_cast<uint64_t>(arg)),
                    hash(0), t_id(T_ID_BOOL){}

    Item(void* arg) : val(std::bit_cast<uint64_t>(arg)),
                    hash(0), t_id(T_ID_NULL){}

    template <std::integral T>
    Item(const T& x) : val(x), hash(0), t_id(T_ID_INT) {}

    template <std::floating_point T>
    Item(const T& x) : val(std::bit_cast<uint64_t>(double(x))),
                       hash(0), t_id(T_ID_FLOAT) {}

    Item(const std::string_view& arg);
    Item(const char* data, size_t _length=-1);

    Item(Fact* arg) : val(std::bit_cast<uint64_t>(arg)),
                    hash(0), t_id(T_ID_FACT){}


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
