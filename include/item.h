#ifndef _ITEM_H_
#define _ITEM_H_

#include <string>
#include <cstdint>
#include <string_view>

struct Item {
    uint64_t val;
    uint64_t hash;
    uint16_t t_id;
    uint16_t pad[3];
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


Item str_to_item(std::string arg);
Item int_to_item(int64_t arg);
Item float_to_item(double arg);

Item to_item(char* arg);
Item to_item(const char* arg);
Item to_item(std::string arg);
Item to_item(int arg);
Item to_item(long arg);
Item to_item(unsigned arg);
Item to_item(double arg);
Item to_item(float arg);
Item to_item(Item arg);

bool item_get_bool(Item item);
int64_t item_get_int(Item item);
double item_get_float(Item item);
std::string_view item_get_string(Item item);

std::string repr_item(Item& item);
std::string to_string(Item& item);

#endif // _ITEM_H_