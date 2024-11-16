#include "../include/helpers.h"
#include "../include/types.h"
#include "../include/item.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/fact.h"
#include "../include/var.h"
#include <bit>
#include <sstream>
#include <functional>


// extern "C" Item empty_item() {
//     Item item;
//     item.val = 0;
//     item.hash = 0;
//     item.t_id = 0;
//     return item;
// }


// Item str_to_item(const std::string_view& arg) {
//     // cout << "SV str_to_item " << arg.length() << endl;
//     // cout << uint64_t(-1) << arg.length() << endl;
//     // cout << "BEFORE INTERN" << endl;
//     InternStr intern_str = intern(arg);
//     // std::string_view intern_str = tup.first;
//     // uint64_t hash = tup.second;
//     // cout << "AFTER INTERN" << endl;
//     const char* data = intern_str.data();

//     UnicodeItem item;
//     item.data = data;
//     item.hash = intern_str.hash;
//     item.t_id = T_ID_STR;
//     item.kind = 1; // TODO
//     item.is_ascii = 1; // TODO
//     item.length = intern_str.length();

//     Item generic_item = std::bit_cast<Item>(item);
//     // cout << "STR_TO_ITEM: " << item.t_id << ", " << generic_item.t_id << endl;
//     return generic_item;
// }

// extern "C" Item str_to_item(const char* data, size_t length) {
//     // cout << "CHAR str_to_item " << length << endl;
//     if(length == size_t(-1)){
//         length = std::strlen(data);
//     }
//     // cout << "CHAR str_to_item " << length << endl;
//     std::string_view sv = std::string_view(data, length);

//     // cout << "CHAR str_to_item " << length << endl;

//     // cout << sv << endl;
//     return str_to_item(sv);
// }

// extern "C" Item bool_to_item(bool arg) {
//     Item item;
//     item.val = static_cast<uint64_t>(arg);
//     item.hash = CREHash{}(arg);
//     item.t_id = T_ID_BOOL;

//     // cout << "BOOL TO ITEM: " << item.t_id << endl;
//     return item;
// }

// extern "C" Item int_to_item(int64_t arg) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(arg);
//     item.hash = CREHash{}(arg);
//     item.t_id = T_ID_INT;

//     // cout << "INT TO ITEM: " << item.t_id << endl;
//     return item;
// }

// extern "C" Item float_to_item(double arg) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(arg);
//     item.hash = CREHash{}(arg);
//     item.t_id = T_ID_FLOAT;
//     // cout << "FLOAT TO ITEM: " << item.t_id << endl;
//     return item;
// }

// extern "C" Item opaque_to_item(void* arg) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(arg);
//     item.hash = 0;
//     item.t_id = T_ID_NULL;
//     // cout << "FLOAT TO ITEM: " << item.t_id << endl;
//     return item;
// }


// ---------------------------------------
// Item::Item():
//     val(0), hash(0), t_id(0), pad(0){
// }

Item::Item(const std::string_view& arg) {
    // cout << "SV str_to_item " << arg.length() << endl;
    // cout << uint64_t(-1) << arg.length() << endl;
    // cout << "BEFORE INTERN" << endl;
    InternStr intern_str = intern(arg);
    // std::string_view intern_str = tup.first;
    // uint64_t hash = tup.second;
    // cout << "AFTER INTERN" << endl;
    const char* data = intern_str.data();

    UnicodeItem item;
    item.data = data;
    item.hash = intern_str.hash;
    item.t_id = T_ID_STR;
    item.kind = 1; // TODO
    item.is_ascii = 1; // TODO
    item.length = intern_str.length();

    *this = std::bit_cast<Item>(item);

    // cout << "ME: " << *this << ", " << uint64_t(data) << endl;
    // Item generic_item = std::bit_cast<Item>(item);
    // cout << "STR_TO_ITEM: " << item.t_id << ", " << generic_item.t_id << endl;
    // return generic_item;
}



Item::Item(const char* data, size_t _length) {
    // cout << "CHAR str_to_item " << length << endl;
    if(_length == size_t(-1)){
        _length = std::strlen(data);
    }
    // cout << "CHAR str_to_item " << length << endl;
    std::string_view sv = std::string_view(data, _length);
    *this = Item(sv);

    // cout << "CHAR str_to_item " << length << endl;

    // cout << sv << endl;
    // return str_to_item(sv);
}

Item::Item(Fact* x, bool _is_ref) : 
            val(std::bit_cast<uint64_t>(x)),
      hash(0), t_id(T_ID_FACT),
      is_ref(_is_ref), borrows(0), pad(0)
{};

Item::Item(Var* x) : val(std::bit_cast<uint64_t>(x)),
                    hash(0), t_id(T_ID_FACT), 
                    is_ref(0), borrows(1), pad(0) 
{
// TODO: should figure out how to do this with move semantics
//   to avoid pointless increfs
    x->inc_ref(); 
};

// Item::Item(bool arg) :
//     val(static_cast<uint64_t>(arg)),
//     hash(0),
//     // hash(CREHash{}(arg)),
//     t_id(T_ID_BOOL) {
// }

// Item::Item(int32_t arg) :
//     val(std::bit_cast<uint64_t>(int64_t(arg))),
//     hash(0),
//     // hash(CREHash{}(int64_t(arg))),
//     t_id(T_ID_INT){
// }

// Item::Item(int64_t arg) :
//     val(std::bit_cast<uint64_t>(arg)),
//     hash(0),
//     // hash(CREHash{}(arg)),
//     t_id(T_ID_INT){
// }

// Item::Item(double arg) :
//     val(std::bit_cast<uint64_t>(arg)),
//     hash(0),
//     // hash(CREHash{}(arg)),
//     t_id(T_ID_FLOAT){
// }

// Item::Item(void* arg) :
//     val(std::bit_cast<uint64_t>(arg)),
//     hash(0),
//     t_id(T_ID_NULL) {
// }


// --------------------------------------



// Item to_item(const char* arg, size_t length) {return str_to_item(arg, length); }
// Item to_item(const std::string_view& arg) {return str_to_item(arg); }
// Item to_item(std::nullptr_t arg) { return opaque_to_item(arg); }
// // Item to_item(void* arg) { return opaque_to_item(arg); }
// Item to_item(bool arg) { return bool_to_item(arg); }
// Item to_item(int32_t arg) { return int_to_item(arg); }
// Item to_item(int64_t arg) { return int_to_item(arg); }
// Item to_item(uint32_t arg) { return int_to_item(arg); }
// Item to_item(uint64_t arg) { return int_to_item(arg); }
// Item to_item(double arg) { return float_to_item(arg); }
// Item to_item(float arg) { return float_to_item(arg); }
// Item to_item(Item arg) { return arg; }


//
// Item to_item(const char* arg, size_t length) {return str_to_item(arg, length); }
// Item to_item(const std::string_view& arg) {return str_to_item(arg); }

//
// Item::Item(std::nullptr_t arg) { return opaque_to_item(arg); }
// // Item to_item(void* arg) { return opaque_to_item(arg); }
// Item::Item(bool arg) { return bool_to_item(arg); }
// Item::Item(int32_t arg) { return int_to_item(arg); }
// Item::Item(int64_t arg) { return int_to_item(arg); }
// Item::Item(uint32_t arg) { return int_to_item(arg); }
// Item::Item(uint64_t arg) { return int_to_item(arg); }
// Item::Item(double arg) { return float_to_item(arg); }
// Item::Item(float arg) { return float_to_item(arg); }




std::string item_to_string(const Item& item) {
    // std::cout << "TO STR: " << item.t_id << std::endl;
    uint16_t t_id = item.t_id;
    std::stringstream ss;
    switch(t_id) {
        case T_ID_NULL:
            ss << "null";
            break;
        case T_ID_BOOL:
            ss << std::boolalpha << item.as_bool();
            break;
        case T_ID_INT:
            ss << item.as_int();
            break;
        case T_ID_FLOAT:
            ss << flt_to_str(item.as_float());
            break;
        case T_ID_STR:
            ss << "'" << std::string(item.as_string()) << "'";
            break;
        case T_ID_FACT:
            {
                Fact* fact = std::bit_cast<Fact*>(item.val);
                if(fact == nullptr){
                    ss << "null";
                    break;
                }else if(fact->type != nullptr){
                    std::string unq_id = fact->get_unique_id();
                    if(!unq_id.empty()){
                        ss << "@" << unq_id;
                        break;
                    }else{
                        cout << "UNIQUE ID FAIL" << endl;
                    }
                }

                if(fact->immutable){
                    ss << fact->to_string(2);
                }else{
                    ss << "<fact f_id=" << fact->f_id << ">";    
                }
            }
            break;
        default:
            ss << "<item t_id=" << t_id << " val=" << item.val << ">";
            // " @" << std::bit_cast<uint64_t>(&item) << ">";     
    }  
    return ss.str();
}



std::string to_string(Item& item) {
    return std::string(item.as_string());
}

std::ostream& operator<<(std::ostream& out, Item item){
    return out << item_to_string(item);
}


// template <typename T = Item>
// uint64_t CREHash::operator()(const T& x) {        
uint64_t hash_item(const Item& x){
    if(x.hash != 0){
        return x.hash;
    }

    uint16_t t_id = x.t_id;
    uint64_t hash; 
    switch(t_id) {
        case T_ID_BOOL:
            hash = CREHash{}(x.as_bool()); break;
        case T_ID_INT:
            hash = CREHash{}(x.as_int()); break;
        case T_ID_FLOAT:
            hash = CREHash{}(x.as_float()); break;
        case T_ID_STR:
            hash = CREHash{}(x.as_string()); break;
        default:
            hash = uint64_t (-1);
    }

    // x.hash = hash;
    return hash;
}

bool Item::operator==(const Item& other) const{
    return (
        this->val == other.val && 
        this->t_id == other.t_id
    );
}
