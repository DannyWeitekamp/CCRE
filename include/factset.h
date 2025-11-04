// #ifndef _CRE_FactSet_H_
// #define _CRE_FactSet_H_
#pragma once 

#include <string>
#include <vector>
#include <bit>
#include <cstdint>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include "item.h"
#include "types.h"
#include "fact.h"
#include "alloc_buffer.h"
#include "ref.h"
// #include "var.h"

namespace cre {

using std::cout;
using std::endl;
using std::vector;


// Forward Declaration
struct FactChange;

//--------------------------------------------------------------
// : FactSet

class FactSet : public CRE_Obj {

public:
    static constexpr uint16_t T_ID = T_ID_FACTSET;
	// -- Members --
	vector<ref<Fact>> facts;
	vector<uint32_t> empty_f_ids;
	uint64_t _size;
    vector<FactChange> change_queue;

	// -- Methods -- 
	FactSet(size_t n_facts=0);
	FactSet(vector<ref<Fact>> facts);
    // ~FactSet();

    inline size_t size() const {return _size;}
    inline size_t capacity() const {return facts.size();}
	uint32_t declare(Fact* fact);
	void retract(uint32_t f_id);
	void retract(Fact* fact);
    vector<ref<Fact>> get_facts();
    ref<Fact> get(uint32_t f_id);

    // ref<Fact> declare_new(FactType* type, const Item* items, size_t n_items);
    // ref<Fact> declare_new(FactType* type, const vector<Item>& items);

    std::string to_string(
        std::string_view format="FactSet{{\n  {}\n}}",
        std::string_view delim="\n  "
    );

    // static ref<FactSet> from_json(char* json_str, size_t length=-1,
    //         const std::string_view& type_attr,
    //         const std::string_view& ref_prefix,
    //         bool copy_buffer=true);

    static ref<FactSet> from_json(
        const std::string_view& json_str,
        const std::string_view& type_attr="type",
        const std::string_view& ref_prefix="@",
        bool copy_buffer=true 
    );//{
        // return FactSet::from_json(json_str.data(), json_str.size(), true);
    // }

    static ref<FactSet> from_json_file(
        const std::string_view& filename,
        const std::string_view& type_attr="type",
        const std::string_view& ref_prefix="@"
    );

    std::string to_json(
        bool unique_id_as_key=false,
        bool doc_as_array=false,
        const std::string_view& type_attr="type",
        const std::string_view& ref_prefix="@"
    );


    // std::string to_string();

    inline void _declare_back(Fact* fact){

        // Equivalent of declare(fs, fact) but don't bother with empties
        // _init_fact(fact, length, type);
        uint32_t f_id = (uint32_t) facts.size();
        fact->f_id = f_id;
        fact->parent = this;
        // cout << "refcount before:" << fact->get_refcount() << endl;
        facts.push_back(fact);
        // cout << "refcount after:" << fact->get_refcount() << endl;
        _size++;   
    }

// -- Iterator --
    class Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ref<Fact>;
        using pointer           = ref<Fact>*;
        using reference         = ref<Fact>&;

    private:
        vector<ref<Fact>>::iterator current;
        vector<ref<Fact>>::iterator end;

    public:
        Iterator(vector<ref<Fact>>::iterator s,
                 vector<ref<Fact>>::iterator e){
            current = s;
            end = e;
        }

        reference operator*() const { return *current; }
        pointer operator->() const { return &*current; }
        Iterator& operator++() { 
            do{
                current++;
            }while(current != end and 
                 *current == nullptr);
            
            return *this; 
        }
        bool operator!=(const Iterator& other) const { return current != other.current; }
        bool operator==(const Iterator& other) const { return current == other.current; }
    };

    Iterator begin() { return  Iterator(facts.begin(), facts.end());}
    Iterator end() { return  Iterator(facts.end(), facts.end());}


    vector<uint8_t> long_hash_bytes(size_t byte_width=24);
    std::string long_hash_string(size_t byte_width=24);



};

std::ostream& operator<<(std::ostream& out, FactSet* fs);

bool is_declared(Fact* fact);
void assert_undeclared(FactSet* fs, Fact* fact);
uint32_t declare(FactSet* fs, Fact* fact);
void retract_f_id(FactSet* fs, uint32_t f_id);
void retract(FactSet* fs, Fact* fact);
void fs_dtor(FactSet* fs);
// extern "C" FactSet* FactSet_from_json(char* json_str, size_t length=-1, bool copy_buffer=true);
// extern "C" FactSet* FactSet_from_json_file(const char* json);
// extern "C" char* FactSet_to_json(FactSet*);




//--------------------------------------------------------------
// : FactSetBuilder



class FactSetBuilder{
public:
    // --- Members ---
    ref<AllocBuffer> alloc_buffer;
    ref<FactSet> fact_set;

    // --- Methods ---
    FactSetBuilder(size_t size=0, size_t buffer_size=0);

    // inline Fact* next_empty(size_t size){
    //     Fact* fact = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size));
    //     fact = new (fact) Fact(size, nullptr, false);
    //     // fact->alloc_buffer = alloc_buffer;
    //     // fact->alloc_buffer->inc_ref();
    //     return fact;
    //     // uint8_t* next_head = buff.head + sizeof(Fact) + size * sizeof(Item);    

    //     // if(next_head <= buff.end){
    //     //     fact = (Fact*) buff.head;
    //     //     buff.head = next_head;
    //     //     // cout << sizeof(Fact) << endl;
    //     //     // cout << "Buff: " << uint64_t(buff.head) << " " << endl; 
    //     // }else{
    //     //     fact = empty_untyped_fact(size);
    //     //     // cout << "ALLOCED! " << endl; 
    //     // }
    //     // fact->length = size;
    //     // fact->type = NULL;
        
    // }

    // inline ref<Fact> new_var(
    //         const Item& _alias,
    //         CRE_Type* _type=nullptr,
    //         DerefInfo* _deref_infos=nullptr,
    //         size_t _length=0
    //     ){

    //     bool did_malloc = false;
    //     AllocBuffer* alloc_buffer = builder.alloc_buffer;
    //     Var* var_addr = (Var*) alloc_buffer->alloc_bytes(SIZEOF_VAR(0), did_malloc);
        
    //     CRE_Type* var_type = fact->type == nullptr ? cre_Fact : fact->type
    //     ref<Var> var = new (var_addr) Var(_alias, _type);


    //     uint32_t size =_resolve_fact_len(length, type);
    //     bool did_malloc = false;
    //     Fact* fact_addr = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size), did_malloc);
    //     ref<Fact> fact = new (fact_addr) Fact(size, type, immutable);

    //     // cout << "refcount: " << fact->get_refcount() << endl;
    //     // cout << "did_malloc:" << did_malloc << endl;
    //     if(!did_malloc){
    //         fact->alloc_buffer = alloc_buffer;
    //         fact->alloc_buffer->inc_ref();
    //     }
    //     return fact;
    // }

    inline ref<Fact> alloc_fact(FactType* type,
                               uint32_t length=0){
        return cre::alloc_fact(type, length, alloc_buffer);
        // length = _resolve_fact_len(type, length);


        // cre::alloc_fact()
        // bool did_malloc = false;
        // Fact* fact_addr = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(length), did_malloc);
        // ref<Fact> fact = new (fact_addr) Fact(type, length);

        // if(!did_malloc){
        //     fact->control_block->alloc_buffer = alloc_buffer;
        //     fact->control_block->alloc_buffer->inc_ref();
        // }

        // // fact->hash = ;

        // return fact;
    }

    inline ref<Fact> empty_fact(FactType* type,
                               size_t length=0){
        return cre::empty_fact(type, length, alloc_buffer);
        // ref<Fact> fact = this->alloc_fact(type, length);
        // _zfill_fact(fact, 0, fact->length);
        // return fact;
    }

    // inline ref<Fact> new_fact

    template<std::derived_from<Item> ItemOrMbr>
    ref<Fact> new_fact(FactType* __restrict  type,
                          const ItemOrMbr* __restrict items,
                          uint32_t n_items,
                          bool immutable=false){
        return cre::new_fact(type, items, n_items, immutable, alloc_buffer);
        // uint32_t length = n_items;//_resolve_fact_len(n_items, type);
        // ref<Fact> fact = this->alloc_fact(type, n_items);

        // _fill_fact(fact, items, n_items);
        // fact->immutable = immutable;
        // return fact;
    }

    template<std::derived_from<Item> ItemOrMbr>
    inline ref<Fact> new_fact(FactType* __restrict  type,
                          const vector<ItemOrMbr>& items,
                          bool immutable=false){
        return new_fact(type, items.data(), items.size(), immutable);
    }

    template<std::derived_from<Item> ItemOrMbr>
    ref<Fact> declare_new(FactType* __restrict  type,
                          const ItemOrMbr* __restrict items,
                          uint32_t n_items,
                          bool immutable=false){
        ref<Fact> fact = this->new_fact(type, items, n_items, immutable);
        fact_set->_declare_back(fact);
        return fact;
    }

    template<std::derived_from<Item> ItemOrMbr>
    inline ref<Fact> declare_new(FactType* __restrict  type,
                          const vector<ItemOrMbr>& items,
                          bool immutable=false){
        return declare_new(type, items.data(), items.size(), immutable);
    }
};


// ---------------------------------------------------
// : ToFactSetTranslator

template <typename T>
struct ToFactSetTranslator {
    using container_t = T::container_t;
    using dict_t = T::dict_t;
    using list_t = T::list_t;
    using tuple_t = T::tuple_t;
    using obj_t = T::obj_t;
    using obj_ptr_t = T::obj_ptr_t;
    using attr_getter_t = T::attr_getter_t;
    // using container_prefix = T::container_prefix;

    struct FactInfo {
        obj_ptr_t obj;
        FactType* type;
        size_t length;
        size_t byte_offset;
        ref<Fact> fact=nullptr;
        bool in_main_buffer = false;
        bool is_alloced = false;
        bool is_initialized = false;
        bool immutable = false;

        FactInfo(obj_ptr_t obj, FactType* type, size_t length, size_t byte_offset,
                 bool in_main_buffer=false, bool immutable=false) :
            obj(obj), type(type), length(length), byte_offset(byte_offset), 
            in_main_buffer(in_main_buffer), immutable(immutable)
        {};
    };

    // using fact_info_t = std::tuple<obj_ptr_t, FactType*, size_t, size_t>;

    std::string_view type_attr;
    std::string_view ref_prefix;
    HashMap<std::string_view, size_t> fact_map;
    vector<FactInfo> fact_infos;
    FactSetBuilder builder;
    attr_getter_t type_attr_ref;

    ToFactSetTranslator() : 
        type_attr("type"), ref_prefix("@"),
        fact_map({}), fact_infos({}), builder({}),
        type_attr_ref(T::to_attr_getter_t("type")) {
    };

    ToFactSetTranslator(
        const std::string_view& _type_attr="type", 
        const std::string_view& _ref_prefix="@") :
        type_attr(_type_attr), ref_prefix(_ref_prefix),
        fact_map({}), fact_infos({}), builder({}),
        type_attr_ref(T::to_attr_getter_t(_type_attr)){
    };

    static ref<FactSet> apply(
     const container_t& obj,
     const std::string_view& type_attr="type",
     const std::string_view& ref_prefix="@"){
        auto trans = ToFactSetTranslator<T>(type_attr, ref_prefix);
        return trans._to_factset(obj);
    }


    /*!
        Resolve the byte_offset, index, and fact_info for a single
        item in the input container when it is converted to a fact
    */
    void _make_fact_info (
        const std::string_view& key_id,
        const obj_ptr_t val_ptr,
        size_t& index,
        size_t& byte_offset){

        auto& val = T::deref_obj_ptr(val_ptr); 

        Item uid_item;
        int uid_index = -1;
        // if(type != nullptr)
           // uid_index = get_unique_id_index(type);
        
        // Resolve the length, and type, and uid of this container item
        size_t length; 
        FactType* type = nullptr;
        if(T::is_dict(val)){
            auto fact_dict = T::to_dict(val);
            length = T::dict_size(fact_dict);
            
            // "type" keyword ignored when computing length
            bool has_type_attr = T::has_attr(fact_dict, type_attr_ref);
            if(has_type_attr){
                type = T::to_fact_type(T::get_attr(fact_dict, type_attr_ref));
                if(type->members.size() > length){
                    length = type->members.size();
                }
                length -= 1; // Don't count type in the count            
            }
            
            if(type != nullptr){
                length = std::max(length, size_t(type->members.size()));
                uid_index = type->unique_id_index;
                if(uid_index != -1 && uid_index < length){
                    std::string_view unq_attr = type->members[uid_index].name;
                    auto unq_attr_getter = T::to_attr_getter_t(unq_attr);
                    if(T::has_attr(fact_dict, unq_attr_getter)){
                        uid_item = T::to_item(T::get_attr(fact_dict, unq_attr_getter));   
                    }
                }
            }else if(has_type_attr){
                std::string obj_str = "<Could not convert Object to String>";

                try{
                    obj_str = T::obj_to_str(val);//nb::cast<std::string>(nb::str(fact_obj));
                } catch (...) {
                    // pass
                }
                
                std::string error_msg = 
                    "CRE could not resolve FactType of object: " +
                    obj_str +"\n"
                    "  Untyped facts cannot be created from attribute-value containers like Python dicts or JSON objects. " +
                    "Add attribute {'type' : 'type_name',...} to the object, or set the type_attr='my_custom_attr' keyword arg " +
                    "to interpret an available attribute as the object's type. Alternatively an untyped fact can be instantiated from a list or tuple.";
                throw std::invalid_argument(error_msg);
            }
        }else if(T::is_list(val)){
            auto fact_list = T::to_list(val);
            length = T::list_size(fact_list);

            // if(uid_index != -1 && uid_index < length){
            //     uid_item = T::to_item(fact_list[uid_index]);
            // }
        }else if(T::is_tuple(val)){   
            if constexpr(!std::is_same<typename T::list_t, typename T::tuple_t>::value){
                tuple_t fact_tuple = T::to_tuple(val);
                length = fact_tuple.size();

                // if(uid_index != -1 && uid_index < length){
                //     uid_item = T::to_item(fact_tuple[uid_index]);
                // }
            }
        }else{
            throw std::domain_error("Fact item with key " + std::string(key_id) + " is not a dict.");
        }

        // Make a fact_info tuple, and insert into fact_map
        fact_infos.push_back(FactInfo(val_ptr, type, length, byte_offset, true));

        // Insert whatever key_id is
        if(key_id.size() > 0){
            auto [it, inserted] = fact_map.insert({key_id, index});
            if(!inserted){
                throw std::domain_error("Duplicate fact identifier: " + std::string(key_id));
            }    
        }

        if(uid_item.get_t_id() != T_ID_UNDEF){
            std::string_view uid = uid_item._as<std::string_view>();
            if(key_id != uid){
                auto [it, inserted] = fact_map.insert({uid, index});
                if(!inserted){
                    throw std::domain_error("Duplicate fact identifier: " + std::string(key_id));
                }          
            }            
        }
        byte_offset += SIZEOF_FACT(length);
    }

    /*!
        A first pass through the input container to retrieve the 
         indicies and offsets of each fact and the total size of
         the resulting FactSet's buffer. Makes a fact_infos vector
         which is used in a second pass to make the actual FactSet
    */
    void _collect_fact_infos(const container_t& container){
        size_t index = 0;
        size_t byte_offset = 0;

        if(T::is_dict(container)){
            auto d = T::to_dict(container);
            size_t L = T::dict_size(d);
            fact_infos.reserve(L);
            fact_map.reserve(L);

            for (auto itr = T::dict_itr_begin(d); itr != T::dict_itr_end(d); ++itr){
                // auto key_ptr = T::dict_itr_key_ptr(itr);
                // auto val_ptr = T::dict_itr_val_ptr(itr);
                // const auto& [key, val] = *itr;
                // std::string_view fact_id = T::to_string_view(T::deref_obj_ptr(key_ptr));
                // _make_fact_info(fact_id, val_ptr, index, byte_offset);

                const auto& [key, val] = *itr;
                auto val_ptr = T::get_obj_ptr(val);
                std::string_view key_id = T::to_string_view(key);
                _make_fact_info(key_id, val_ptr, index, byte_offset);
                index++;
            }
        }else if(T::is_list(container)) {
            auto lst = T::to_list(container);
            size_t L = T::list_size(lst);
            fact_infos.reserve(L);
            fact_map.reserve(L);

            // auto& [d_start, d_end] = T::list_itr(lst);
            for (auto itr = T::list_itr_begin(lst); itr != T::list_itr_end(lst); ++itr){
                auto val_ptr = T::list_itr_val_ptr(itr);
            // for (auto val : lst) {
                // std::string fact_id = std::to_string(index);
                std::string_view key_id;
                _make_fact_info(key_id, val_ptr, index, byte_offset);
                index++;
            }
        }
        builder = FactSetBuilder(fact_infos.size(), byte_offset);
        // return std::make_tuple(fact_infos, fact_map, byte_offset);
    }

    /*!
        Convert string items that reference other facts to fact Items 
    */

    Item _resolve_possible_fact_ref(Item item, CRE_Type* mbr_type){

        int64_t index = -1;
        if(item.get_t_id() == T_ID_STR){
            std::string_view item_str = item._as<std::string_view>();

            // Reference to another fact
            size_t ref_len = ref_prefix.size();
            std::string_view ref_str = {};

            // If the prefix is present like "@bob"
            if(item_str.length() > 0 && 
               !item_str.compare(0, ref_len, ref_prefix)
               ){
                // Strip the prefix "@bob" -> "bob"
                ref_str = std::string_view(
                    item_str.data()+ref_len,
                    item_str.length()-ref_len
                );

            // If or if the member's type is FactType
            }else if(mbr_type && 
                     mbr_type->t_id == T_ID_FACT){
                ref_str = item_str;
            }

            if(ref_str.size() > 0){
                // cout << std::string(ref_str) << endl;

                auto itr = fact_map.find(ref_str);
                
                 // If the value does not uniquely identify a fact 
                if(itr == fact_map.end()){
                    // See if the string is an integer 
                    char* p;
                    index = strtol(ref_str.data(),&p,10);
                    if(*p != 0 || index < 0 || index > fact_infos.size()){
                        throw std::invalid_argument(
                            "Cannot resolve string identifier \"" + std::string(item_str) + "\" for Fact reference." 
                        );    
                    }
                }else{
                    index = itr->second;
                }
                
                
            }
            
        }else if(item.get_t_id() == T_ID_INT){
            if(mbr_type && mbr_type->t_id == T_ID_FACT){
                index = item._as<int64_t>();
                if(index < 0 || index >= fact_infos.size()){
                    throw std::invalid_argument(
                        "Cannot resolve integer indentifier \"" + std::to_string(index) + "\" for Fact reference." 
                    );    
                }
            }
        }

        // If reference was resolved then set item to the fact it resolves to 
        if(index != -1){
            // size_t offset = fact_infos[index].byte_offset;
            // Fact* fact_ptr = (Fact*)(builder.alloc_buffer->data + offset);
            FactInfo& info = fact_infos[index];
            ref<Fact> fact = _alloc_fact(info);
            if(info.immutable){
                _init_fact(info);
            }
            // cout << "1REFCNT:" << fact->get_refcount() << endl;
            return Item(fact);
        }
        return item;
    }


    std::tuple<int64_t, CRE_Type*, bool> _resolve_mbr_index_type(
        const auto& key, FactType* type){

        

        std::string_view key_str = T::to_string_view(key);
        // Ignore the 'type' member (handled above)
        if(key_str == type_attr){
            return std::make_tuple(-1, nullptr, true);
        }

        // If the key is a number then used it as the index
        char* p;
        int64_t index = strtol(key_str.data(),&p,10);
        if(*p != 0) index = -1;

        // Otherwise get the key's index from the fact type
        if(type != nullptr && index == -1){
            index = type->get_attr_index(key_str);
        }

        // Throw error if member index cannot be resolved
        // cout << "key_str=" << key_str << ", val str=" << T::to_string_view(val) << ", type_attr=" << type_attr << " INDEX:" << index << endl; 
        if(index == -1){
            std::string type_str = type != nullptr ? std::string(type->name) : "NULL";
            std::string error_msg = "Key '" + std::string(key_str) +
                "' is not an integer or named member of fact type '" +
                type_str + "'.";
            throw std::domain_error(error_msg);
        }

        CRE_Type* mbr_type = index == -1 || type == nullptr ? 
                            nullptr : 
                            type->members[index].type;

        return std::make_tuple(index, mbr_type, false);
    }

    ref<Fact> _alloc_fact(FactInfo& fact_info){
        if(!fact_info.is_alloced){
            if(fact_info.in_main_buffer){
                fact_info.fact = builder.alloc_fact(fact_info.type, fact_info.length);
            }else{
                fact_info.fact = alloc_fact(fact_info.type, fact_info.length);
            }    
            fact_info.is_alloced = true;
        }

        return fact_info.fact;
    }

    void _init_fact(FactInfo& fact_info){
        if(fact_info.is_initialized){
            return;
        }

        auto& fact_obj = T::deref_obj_ptr(fact_info.obj);
        FactType* type = fact_info.type;
        Fact* fact = fact_info.fact;
        // fact->type = type;

        if(T::is_dict(fact_obj)){
            // Note: Dict instatiated facts must be zero filled
            //  since there is no gaurentee that all of their fields
            //  were provided. 
            _zfill_fact(fact, 0, fact_info.length);

            auto fact_dict = T::to_dict(fact_obj);
            for (auto itr = T::dict_itr_begin(fact_dict); itr != T::dict_itr_end(fact_dict); ++itr){
                const auto& [key, val] = *itr;
                auto [index, mbr_type, skip] = _resolve_mbr_index_type(key, type);          
                if(skip) continue;
                Item item = T::to_item(val);
                item = this->_resolve_possible_fact_ref(item, mbr_type);
                fact->set(index, std::move(item));
            }
        } else if(T::is_list(fact_obj)){
            auto fact_list = T::to_list(fact_obj);
            int64_t index = 0;
            for (auto itr = T::list_itr_begin(fact_list); itr != T::list_itr_end(fact_list); ++itr){
                // auto val_ptr = T::list_itr_val_ptr(itr);
                try{
                    const auto& val = *itr;
                    Item item = T::to_item(val);
                    item = this->_resolve_possible_fact_ref(item, nullptr);                    
                    fact->set_unsafe(index, std::move(item));
                } catch (const std::exception& e) {
                    _zfill_fact(fact, index, fact_info.length);
                    throw;
                }

                index++;
            }
            if(T::treat_lists_immutable){
                fact->immutable = true;    
            }

        } else if(T::is_tuple(fact_obj)){

        if constexpr(!std::is_same<typename T::list_t, typename T::tuple_t>::value){   
            auto fact_tuple = T::to_tuple(fact_obj);
            int64_t index = 0;
            for (auto itr = T::tuple_itr_begin(fact_tuple); itr != T::tuple_itr_end(fact_tuple); ++itr){
                // auto val_ptr = T::list_itr_val_ptr(itr);
                try{
                    const auto& val = *itr;
                    Item item = T::to_item(val);
                    item = this->_resolve_possible_fact_ref(item, nullptr);
                    fact->set_unsafe(index, std::move(item));    
                } catch (const std::exception& e) {
                    _zfill_fact(fact, index, fact_info.length);
                    throw;
                }
                
                index++;
            }
            fact->immutable = true;
        }

        }else{
            throw std::invalid_argument(T::container_prefix + 
                " \"" + T::obj_to_str(fact_obj) + 
                "\" cannot be converted to Fact.");
        }
        fact->ensure_unique_id();
        builder.fact_set->_declare_back(fact);
        fact_info.is_initialized = true;
        // builder.alloc_buffer->add_ref(fact_infos.size());
    }

    /*!
        Make the FactSet. Implements second pass that consumes fact_infos
    */

    ref<FactSet> _to_factset(const container_t& container){
        this->_collect_fact_infos(container);
        
        for(auto& fact_info : fact_infos){
            _alloc_fact(fact_info);
            _init_fact(fact_info);
        }
        
        return builder.fact_set;
    }
};


// -----------------------------------------------
// FactChange

const uint16_t CHANGE_KIND_INIT = 0;
const uint16_t CHANGE_KIND_DECLARE = 1;
const uint16_t CHANGE_KIND_RETRACT = 2;
const uint16_t CHANGE_KIND_MODIFY = 3;


struct FactChange {
    uint32_t f_id;
    uint16_t a_id;
    uint16_t kind;
    // Item item;
}; 

} // NAMESPACE_END(cre)

// #endif /* _CRE_FactSet_H_ */ 
