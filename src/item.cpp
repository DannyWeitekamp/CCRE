#include "../include/types.h"
#include "../include/item.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/fact.h"
#include <bit>
#include <sstream>
#include <functional>


extern "C" Item empty_item() {
    Item item;
    item.val = 0;
    item.hash = 0;
    item.t_id = 0;
    return item;
}


Item str_to_item(const std::string_view& arg) {
    // cout << "SV str_to_item " << arg.length() << endl;
    // cout << uint64_t(-1) << arg.length() << endl;
    // cout << "BEFORE INTERN" << endl;
    auto tup = intern_ret_hash(arg);
    std::string_view intern_str = tup.first;
    uint64_t hash = tup.second;
    // cout << "AFTER INTERN" << endl;
    const char* data = intern_str.data();

    UnicodeItem item;
    item.data = data;
    item.hash = hash;
    item.t_id = T_ID_STR;
    item.kind = 1; // TODO
    item.is_ascii = 1; // TODO
    item.length = intern_str.length();

    Item generic_item = std::bit_cast<Item>(item);
    // cout << "STR_TO_ITEM: " << item.t_id << ", " << generic_item.t_id << endl;
    return generic_item;
}

extern "C" Item str_to_item(const char* data, size_t length) {
    // cout << "CHAR str_to_item " << length << endl;
    if(length == size_t(-1)){
        length = std::strlen(data);
    }
    // cout << "CHAR str_to_item " << length << endl;
    std::string_view sv = std::string_view(data, length);

    // cout << "CHAR str_to_item " << length << endl;

    // cout << sv << endl;
    return str_to_item(sv);
}

extern "C" Item bool_to_item(bool arg) {
    Item item;
    item.val = static_cast<uint64_t>(arg);
    item.hash = CREHash{}(arg);
    item.t_id = T_ID_BOOL;

    // cout << "BOOL TO ITEM: " << item.t_id << endl;
    return item;
}

extern "C" Item int_to_item(int64_t arg) {
    Item item;
    item.val = std::bit_cast<uint64_t>(arg);
    item.hash = CREHash{}(arg);
    item.t_id = T_ID_INT;

    // cout << "INT TO ITEM: " << item.t_id << endl;
    return item;
}

extern "C" Item float_to_item(double arg) {
    Item item;
    item.val = std::bit_cast<uint64_t>(arg);
    item.hash = CREHash{}(arg);
    item.t_id = T_ID_FLOAT;
    // cout << "FLOAT TO ITEM: " << item.t_id << endl;
    return item;
}

extern "C" Item opaque_to_item(void* arg) {
    Item item;
    item.val = std::bit_cast<uint64_t>(arg);
    item.hash = 0;
    item.t_id = T_ID_NULL;
    // cout << "FLOAT TO ITEM: " << item.t_id << endl;
    return item;
}



Item to_item(const char* arg, size_t length) {return str_to_item(arg, length); }
Item to_item(const std::string_view& arg) {return str_to_item(arg); }
Item to_item(std::nullptr_t arg) { return opaque_to_item(arg); }
// Item to_item(void* arg) { return opaque_to_item(arg); }
Item to_item(bool arg) { return bool_to_item(arg); }
Item to_item(int32_t arg) { return int_to_item(arg); }
Item to_item(int64_t arg) { return int_to_item(arg); }
Item to_item(uint32_t arg) { return int_to_item(arg); }
Item to_item(uint64_t arg) { return int_to_item(arg); }
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

std::string item_to_string(const Item& item) {
    // std::cout << "TO STR: " << item.t_id << std::endl;
    uint16_t t_id = item.t_id;
    std::stringstream ss;
    switch(t_id) {
        case T_ID_NULL:
            ss << "null";
            break;
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
        case T_ID_FACT:
            {
                Fact* fact = std::bit_cast<Fact*>(item.val);
                if(fact == nullptr){
                    ss << "null";
                    break;
                }else if(fact->type != nullptr){
                    std::string unq_id = fact_to_unique_id(fact);
                    if(!unq_id.empty()){
                        ss << "@" << unq_id;
                        break;
                    }else{
                        cout << "UNIQUE ID FAIL" << endl;
                    }
                }

                if(fact->immutable){
                    ss << fact_to_string(fact, 2);
                }else{
                    ss << "<fact f_id=" << fact->f_id << ">";    
                }
            }
            break;
        default:
            ss << "<item t_id=" << t_id << ">";
            // " @" << std::bit_cast<uint64_t>(&item) << ">";     
    }  
    return ss.str();
}

std::string to_string(Item& item) {
    return item_to_string(item);
}

std::ostream& operator<<(std::ostream& out, Item item){
    return out << item_to_string(item);
}


uint64_t CREHash::operator()(Item& x) {        
    if(x.hash != 0){
        return x.hash;
    }

    uint16_t t_id = x.t_id;
    uint64_t hash; 
    switch(t_id) {
        case T_ID_BOOL:
            hash = CREHash::operator()(item_get_bool(x)); break;
        case T_ID_INT:
            hash = CREHash::operator()(item_get_int(x)); break;
        case T_ID_FLOAT:
            hash = CREHash::operator()(item_get_float(x)); break;
        case T_ID_STR:
            hash = CREHash::operator()(item_get_string(x)); break;
        default:
            hash = uint64_t (-1);
    }

    x.hash = hash;
    return hash;
}
