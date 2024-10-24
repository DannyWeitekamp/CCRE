#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include <cstdint>
#include <iostream>
#include <string_view>
#include "../include/types.h"

// Externally Defined Forward Declares
class Fact;

struct Item {
    uint64_t val;
    uint64_t hash;
    uint16_t t_id;
    uint16_t pad[3];

    bool operator==(const Item& other) const;

    Item();
    Item(std::nullptr_t arg);
    Item(bool arg);
    Item(void* arg);

    template <std::integral T>
    Item(const T& x) : val(x), hash(0), t_id(T_ID_INT) {}

    template <std::floating_point T>
    Item(const T& x) : val(x), hash(0), t_id(T_ID_FLOAT) {}

    Item(const std::string_view& arg);
    Item(const char* data, size_t _length);
    
};

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


extern "C" Item empty_item();
Item str_to_item(const std::string_view& arg);
extern "C" Item str_to_item(const char* data, size_t length);
extern "C" Item opaque_to_item(void* arg);
extern "C" Item nullptr_to_item(std::nullptr_t arg);
extern "C" Item bool_to_item(bool arg);
extern "C" Item int_to_item(int64_t arg);
extern "C" Item float_to_item(double arg);



Item to_item(const char* arg, size_t length=-1);
Item to_item(const std::string_view& arg);
// Item to_item(const std::string& arg) ;
Item to_item(std::nullptr_t arg);
// Item to_item(void* arg);
Item to_item(bool arg);
Item to_item(int32_t arg);
Item to_item(int64_t arg);
Item to_item(uint32_t arg);
Item to_item(uint64_t arg);
Item to_item(double arg);
Item to_item(float arg);
Item to_item(Item arg);

Item to_item(Fact* arg);

std::ostream& operator<<(std::ostream& out, Item item);

bool item_get_bool(Item item);
int64_t item_get_int(Item item);
double item_get_float(Item item);
std::string_view item_get_string(Item item);
Fact* item_get_fact(const Item& item);

std::string item_to_string(const Item& item);
std::string to_string(Item& item);

#endif // _ITEM_H_
