#include "../include/helpers.h"
#include "../include/types.h"
#include "../include/item.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/fact.h"
#include "../include/var.h"
#include "../include/func.h"
#include <bit>
#include <sstream>
#include <functional>

namespace cre {

// extern "C" Item empty_item() {
//     Item item;
//     item.val = 0;
//     item.hash = 0;
//     item.get_t_id() = 0;
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
//     item.get_t_id() = T_ID_STR;
//     item.kind = 1; // TODO
//     item.is_ascii = 1; // TODO
//     item.length = intern_str.length();

//     Item generic_item = std::bit_cast<Item>(item);
//     // cout << "STR_TO_ITEM: " << item.get_t_id() << ", " << generic_item.get_t_id() << endl;
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
//     item.get_t_id() = T_ID_BOOL;

//     // cout << "BOOL TO ITEM: " << item.get_t_id() << endl;
//     return item;
// }

// extern "C" Item int_to_item(int64_t arg) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(arg);
//     item.hash = CREHash{}(arg);
//     item.get_t_id() = T_ID_INT;

//     // cout << "INT TO ITEM: " << item.get_t_id() << endl;
//     return item;
// }

// extern "C" Item float_to_item(double arg) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(arg);
//     item.hash = CREHash{}(arg);
//     item.get_t_id() = T_ID_FLOAT;
//     // cout << "FLOAT TO ITEM: " << item.get_t_id() << endl;
//     return item;
// }

// extern "C" Item opaque_to_item(void* arg) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(arg);
//     item.hash = 0;
//     item.get_t_id() = T_ID_NULL;
//     // cout << "FLOAT TO ITEM: " << item.get_t_id() << endl;
//     return item;
// }


// ---------------------------------------
// Item::Item():
//     val(0), hash(0), t_id(0), pad(0){
// }

Item::Item(const std::string_view& arg) :
    val(0), t_id(T_ID_UNDEF),
    meta_data(0), val_kind(VALUE), pad(0) 
 {
    // cout << "SV str_to_item " << arg.length() << endl;
    // cout << uint64_t(-1) << arg.length() << endl;
    // cout << "BEFORE INTERN" << endl;
    InternStr intern_str = intern(arg);
    // std::string_view intern_str = tup.first;
    // uint64_t hash = tup.second;
    // cout << "AFTER INTERN" << endl;
    // const char* data = intern_str.data();

    // UnicodeItem item;
    data = intern_str.data();
    length = uint32_t(intern_str.length());
    // item.hash = intern_str.hash;
    t_id = T_ID_STR;
    meta_data = 0; // TODO
    val_kind = VALUE; 
    
    // TODO
    // item.kind = 1; 
    // item.is_ascii = 1; // TODO
    

    // *this = std::bit_cast<Item>(item);

    // cout << "ME: " << *this << ", " << uint64_t(data) << endl;
    // Item generic_item = std::bit_cast<Item>(item);
    // cout << "STR_TO_ITEM: " << item.get_t_id() << ", " << generic_item.get_t_id() << endl;
    // return generic_item;
}



Item::Item(const char* data, size_t _length) :
    val(0), t_id(T_ID_UNDEF),
    meta_data(0), val_kind(VALUE), pad(0) 
{
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


// // -- Fact -> Item ---
// Item::Item(Fact* x) : 
//       ptr((void *) x),
//       t_id(T_ID_FACT),
//       meta_data(0), 
//       val_kind(RAW_POINTER),
//       pad(0)

//       // is_ref(_is_ref == 0xFF ? !x->immutable : _is_ref), 
       
// {
//     // x->inc_ref();
// };


// Item::Item(ref<Fact> x) : 
//       ptr((void *) x.get()),
//       t_id(T_ID_FACT),
//       meta_data(0), 
//       val_kind(STRONG_PTR),
//       pad(0)
// {
//     x->inc_ref();
// };


// Item::Item(wref<Fact> x) : 
//       ptr((void *) x.get()),
//       t_id(T_ID_FACT),
//       meta_data(0), 
//       val_kind(WEAK_PTR),
//       pad(0)
// {
//     x->inc_ref();
// };



// // -- Var -> Item ---
// Item::Item(Var* x) : val(std::bit_cast<uint64_t>(x)),
//                     t_id(T_ID_VAR), 
//                     is_ref(0), borrows(1), pad(0) 
// {
// // TODO: should figure out how to do this with move semantics
// //   to avoid pointless increfs
//     // x->inc_ref(); 
// };

// Item::Item(ref<Var> x) : Item((Var*) x)
// {};


// // -- Func -> Item ---
// Item::Item(Func* x) : val(std::bit_cast<uint64_t>(x)),
//                     t_id(T_ID_FUNC), 
//                     is_ref(0), borrows(1), pad(0) 
// {
// // TODO: should figure out how to do this with move semantics
// //   to avoid pointless increfs
//     // x->inc_ref(); 
// };

// Item::Item(ref<Func> x) : Item((Func*) x)
// {};


void Item::borrow() const {
    if(is_ref()){
        if(is_wref()){
            ControlBlock* cb = (ControlBlock*) ptr;
            cb->inc_wref();
            // throw std::runtime_error("HEY!");
            // cout << this->to_string() << " t_id=" << t_id << " BORROW WEAK: " << cb->get_wrefcount() << endl;
        }else{
            ((CRE_Obj*) ptr)->inc_ref();
            // cout << this->to_string() << " t_id=" << t_id << " BORROW STRONG: " << ((CRE_Obj*) ptr)->get_refcount() << endl;
        }
    }
}

void Item::release() const {
    // cout << "RELEASE:" << is_ref() << ", " << is_wref() << endl;
    if(is_ref()){
        if(is_wref()){
            ControlBlock* cb = (ControlBlock*) ptr;
            cb->dec_wref();
        }else{
            ((CRE_Obj*) val)->dec_ref();
        }
    }
}

// void Item::destroy(){
    
// }

// val(std::bit_cast<uint64_t>(x.get())),
//                     t_id(T_ID_VAR), 
//                     is_ref(0), borrows(1), pad(0) 
// {
// // TODO: should figure out how to do this with move semantics
// //   to avoid pointless increfs
//     x->inc_ref(); 
// };



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
    
    uint16_t t_id = item.get_t_id();
    // std::cout << "TO STR: " << t_id << std::endl;
    std::stringstream ss;
        
    bool known_type = true;
    if(item.is_value()){
        switch(t_id) {

        case T_ID_UNDEF:
            ss << "Undef";
            break;
        case T_ID_NONE:
            ss << "None";
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
        default:
            known_type = false;
        }
    }else{
        bool expired = item.is_expired();
        if(expired){
            ControlBlock* cb = item.get_cb();
            if(cb->unique_id == ""){
                ss << "expired[??]";   
            }else{
                ss << "expired[@" << cb->unique_id << "]";
            }
            return ss.str();
        }
        switch(t_id) {

        case T_ID_FACT:
            {
                Fact* fact = item.as_fact();
                        
                if(fact->type != nullptr){
                    std::string unq_id = fact->get_unique_id();

                    if(!unq_id.empty()){
                        ss << "@" << unq_id;
                        break;
                    }else{
                        cout << "UNIQUE ID FAIL" << endl;
                    }
                }

                // TODO: Needs testing
                if( uint64_t(&item) >= uint64_t(fact) + sizeof(Fact) && 
                    uint64_t(&item) < uint64_t(fact) + SIZEOF_FACT(fact->length)){
                    ss << "self";
                    break;
                }

                if(fact->immutable){
                    ss << fact->to_string(2);
                }else{
                    ss << "<fact f_id=" << fact->f_id << ">";    
                }
                break;
            }
        case T_ID_VAR:
            ss << item.as_var();
            break;
        case T_ID_FUNC:
            ss << item.as_func();
            break;
        default:
            known_type = false;
            // " @" << std::bit_cast<uint64_t>(&item) << ">"; 
    }}
    if(!known_type){
        ss << "<item t_id=" << t_id << " val=" << item.val << ">";
    }

    return ss.str();
}



std::string to_string(Item& item){
    return std::string(item.as_string());
}

std::string Item::to_string() const {
    return item_to_string(*this);
}

std::ostream& operator<<(std::ostream& out, Item item){
    return out << item_to_string(item);
}


// template <typename T = Item>
// uint64_t CREHash::operator()(const T& x) {        
uint64_t hash_item(const Item& x){
    // if(x.hash != 0){
    //     return x.hash;
    // }

    uint16_t t_id = x.get_t_id();
    uint64_t hash; 
    switch(t_id) {
        case T_ID_UNDEF:
            hash = 0; break;
        case T_ID_NONE:
            hash = 0; break;
        case T_ID_BOOL:
            hash = CREHash{}(x.as_bool()); break;
        case T_ID_INT:
            hash = CREHash{}(x.as_int()); break;
        case T_ID_FLOAT:
            hash = CREHash{}(x.as_float()); break;
        case T_ID_STR:
            hash = CREHash{}(x.as_string()); break;
        case T_ID_FACT:
            {
                Fact* fact = x.as_fact();
                if(fact != nullptr && !x.is_ref()){
                    hash = CREHash{}(fact); 
                }else{
                    hash = 0;
                }
            }
            break;
        case T_ID_VAR:
            {
                Var* var = x.as_var();
                if(var != nullptr){
                    hash = CREHash{}(var); 
                }else{
                    hash = 0;
                }
            }
            break;
        default:
            // cout << "Warning Default Hash, t_id=" << x.get_t_id() << ", val=" << x.val << endl;
            hash = uint64_t (0);
    }

    // x.hash = hash;
    return hash;
}

bool Item::operator==(const Item& other) const{
    return (
        this->val == other.val && 
        this->t_id == other.get_t_id()
    );
}

} // NAMESPACE_END(cre)
