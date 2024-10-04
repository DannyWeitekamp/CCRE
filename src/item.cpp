#include "../include/types.h"
#include "../include/item.h"
#include "../include/unicode.h"
#include <bit>
#include <sstream>
#include <functional>


Item str_to_item(std::string arg) {
    auto tup = intern(arg);
    const char* data = std::get<1>(tup);

    UnicodeItem item;
    item.data = data;
    item.hash = std::hash<std::string>{}(arg);
    item.kind = 1; // TODO
    item.is_ascii = 1; // TODO
    item.t_id = T_ID_STR;
    item.length = arg.length();

    return std::bit_cast<Item>(item);
}

Item int_to_item(int64_t arg) {
    Item item;
    item.val = std::bit_cast<uint64_t>(arg);
    item.hash = std::bit_cast<uint64_t>(arg);
    item.t_id = T_ID_INT;
    return item;
}

Item float_to_item(double arg) {
    Item item;
    item.val = std::bit_cast<uint64_t>(arg);
    item.hash = std::bit_cast<uint64_t>(arg);
    item.t_id = T_ID_FLOAT;
    return item;
}

Item to_item(char* arg) { return str_to_item(arg); }
Item to_item(const char* arg) { return str_to_item(arg); }
Item to_item(std::string arg) { return str_to_item(arg); }
Item to_item(int arg) { return int_to_item(arg); }
Item to_item(long arg) { return int_to_item(arg); }
Item to_item(unsigned arg) { return int_to_item(arg); }
Item to_item(double arg) { return float_to_item(arg); }
Item to_item(float arg) { return float_to_item(arg); }
Item to_item(Item arg) { return arg; }

bool item_get_bool(Item item) {
    return (bool) item.val;
}

int64_t item_get_int(Item item) {
    return std::bit_cast<int64_t>(item.val);
}

double item_get_float(Item item) {
    return std::bit_cast<double>(item.val);
}

std::string_view item_get_string(Item item) {
    UnicodeItem ut = std::bit_cast<UnicodeItem>(item);
    return std::string_view(ut.data, ut.length);
}

std::string repr_item(Item& item) {
    uint16_t t_id = item.t_id;
    std::stringstream ss;
    switch(t_id) {
        case T_ID_BOOL:
            ss << std::boolalpha << item_get_bool(item);
            break;
        case T_ID_INT:
            ss << item_get_int(item);
            break;
        case T_ID_FLOAT:
            ss << std::to_string(item_get_float(item));
            break;
        case T_ID_STR:
            ss << "'" << item_get_string(item) << "'";
            break;
        default:
            ss << "<item t_id=" << t_id << ">";
            // " @" << std::bit_cast<uint64_t>(&item) << ">";     
    }  
    return ss.str();
}

std::string to_string(Item& item) {
    return repr_item(item);
}
