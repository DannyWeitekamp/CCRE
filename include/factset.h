#ifndef _CRE_FactSet_H_
#define _CRE_FactSet_H_

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

using std::cout;
using std::endl;


// Forward Declaration
struct FactChange;

//--------------------------------------------------------------
// : FactSet

class FactSet : public CRE_Obj {

public:
	// -- Members --
	std::vector<ref<Fact>> facts;
	std::vector<uint32_t> empty_f_ids;
	uint64_t _size;
    std::vector<FactChange> change_queue;

	// -- Methods -- 
	FactSet(size_t n_facts=0);
	FactSet(std::vector<ref<Fact>> facts);
    // ~FactSet();

    inline size_t size() const {return _size;}
	uint32_t declare(Fact* fact);
	void retract(uint32_t f_id);
	void retract(Fact* fact);
    std::vector<ref<Fact>> get_facts();

    ref<Fact> add_fact(FactType* type, const Item* items, size_t n_items);
    ref<Fact> add_fact(FactType* type, const std::vector<Item>& items);

    std::string to_string(
        std::string_view format="FactSet{{\n  {}\n}}",
        std::string_view delim="\n  "
    );

    static ref<FactSet> from_json(char* json_str, size_t length=-1, bool copy_buffer=true);
    static ref<FactSet> from_json_file(const char* json);
    static char* to_json(FactSet* fs);


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
        std::vector<ref<Fact>>::iterator current;
        std::vector<ref<Fact>>::iterator end;

    public:
        Iterator(std::vector<ref<Fact>>::iterator s,
                 std::vector<ref<Fact>>::iterator e){
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


};

std::ostream& operator<<(std::ostream& out, FactSet* fs);

extern "C" bool is_declared(Fact* fact);
extern "C" void assert_undeclared(FactSet* fs, Fact* fact);
extern "C" uint32_t declare(FactSet* fs, Fact* fact);
extern "C" void retract_f_id(FactSet* fs, uint32_t f_id);
extern "C" void retract(FactSet* fs, Fact* fact);
extern "C" void fs_dtor(FactSet* fs);
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

    inline Fact* next_empty(size_t size){
        Fact* fact = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size));
        fact = new (fact) Fact(size, nullptr, false);
        // fact->alloc_buffer = alloc_buffer;
        // fact->alloc_buffer->inc_ref();
        return fact;
        // uint8_t* next_head = buff.head + sizeof(Fact) + size * sizeof(Item);    

        // if(next_head <= buff.end){
        //     fact = (Fact*) buff.head;
        //     buff.head = next_head;
        //     // cout << sizeof(Fact) << endl;
        //     // cout << "Buff: " << uint64_t(buff.head) << " " << endl; 
        // }else{
        //     fact = empty_untyped_fact(size);
        //     // cout << "ALLOCED! " << endl; 
        // }
        // fact->length = size;
        // fact->type = NULL;
        
    }

    inline ref<Fact> add_empty(size_t length,
                               FactType* type,
                               bool immutable=false){
        uint32_t size =_resolve_fact_len(length, type);
        bool did_malloc = false;
        Fact* fact_addr = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size), did_malloc);
        ref<Fact> fact = new (fact_addr) Fact(size, type, immutable);

        // cout << "refcount: " << fact->get_refcount() << endl;
        // cout << "did_malloc:" << did_malloc << endl;
        if(!did_malloc){
            fact->alloc_buffer = alloc_buffer;
            fact->alloc_buffer->inc_ref();
        }
        return fact;
    }

    inline ref<Fact> add_fact(FactType* __restrict  type,
                          const Item* __restrict items,
                          uint32_t n_items,
                          bool immutable=false){

        uint32_t size =_resolve_fact_len(n_items, type);
        // Fact* fact = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size));
        // fact = new (fact) Fact(size, type, immutable);
        // fact->alloc_buffer = alloc_buffer;
        // fact->alloc_buffer->inc_ref();
        ref<Fact> fact = add_empty(size, type, immutable);

        // cout << "refcount: " << fact->get_refcount() << endl;
        memcpy(((uint8_t*)fact.get()) + sizeof(Fact), (uint8_t*) items, size * sizeof(Item));
        fact_set->_declare_back(fact);
        return fact;
    }

    inline ref<Fact> add_fact(FactType* __restrict  type,
                          const std::vector<Item>& items,
                          bool immutable=false){
        return add_fact(type, items.data(), items.size(), immutable);
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
    using attr_getter_t = T::attr_getter_t;

    std::string_view type_attr;
    std::string_view ref_prefix;
    HashMap<std::string_view, size_t> fact_map;
    std::vector<std::tuple<obj_t, FactType*, size_t, size_t>> fact_infos;
    FactSetBuilder builder;

    ToFactSetTranslator() : 
        type_attr("type"), ref_prefix("@"),
        fact_map({}), fact_infos({}), builder({}) {

    };
    ToFactSetTranslator(
        const std::string_view& _type_attr="type", 
        const std::string_view& _ref_prefix="@") :
        type_attr(_type_attr), ref_prefix(_ref_prefix),
        fact_map({}), fact_infos({}), builder({}){
    };

    static ref<FactSet> apply(
     container_t obj,
     const std::string_view& type_attr="type",
     const std::string_view& ref_prefix="@"){
        auto trans = ToFactSetTranslator<T>(type_attr, ref_prefix);
        return trans._to_factset(obj);
    }

    void _collect_fact_infos(container_t container){
        attr_getter_t ref_type_obj = T::to_attr_getter_t(type_attr);

        // fact_map = {};
        // fact_infos = {};
        size_t index = 0;
        size_t byte_offset = 0;

        // Lambda Function
        auto make_fact_info = [&](
            const std::string_view& fact_id,
            obj_t val
        ){
            // TODO: Handle integer keys
            // if(nb::isinstance<nb::int>(key)):
                // std::to_string()
            
            size_t length; 
            FactType* type = nullptr;

            // cout << "B" << endl;
            if(T::is_dict(val)){
                dict_t fact_dict = T::to_dict(val);
                length = fact_dict.size();
                
                // cout << "C" << endl;

                if(T::has_attr(fact_dict, ref_type_obj)){
                    // cout << "D" << endl;
                    // nb::print(fact_dict[ref_type_obj]);
                    type = T::to_fact_type(T::get_attr(fact_dict,ref_type_obj));
                    // cout << "E" << endl;
                    if(type->members.size() > length){
                        length = type->members.size();
                    }
                    // cout << "F" << endl;
                    length -= 1; // Don't count type in the count            
                }
                
                if(type != nullptr){
                    length = std::max(length, size_t(type->members.size()));
                }
            }else if(T::is_list(val)){
                list_t fact_list = T::to_list(val);
                length = fact_list.size();
            }else if(T::is_tuple(val)){   
                if constexpr(T::to_tuple){
                    tuple_t fact_tuple = T::to_tuple(val);
                    length = fact_tuple.size();
                }
            }else{
                throw std::runtime_error("Fact item with key " + std::string(fact_id) + " is not a dict.");
            }

            // cout << "C" << endl;
            fact_infos.push_back({val, type, length, byte_offset});
            auto [it, inserted] = fact_map.insert({fact_id, index});
            // cout << fact_id << endl;
            if(!inserted){
                throw std::runtime_error("Duplicate fact identifier: " + std::string(fact_id));
            }
            byte_offset += SIZEOF_FACT(length);//sizeof(Fact) + sizeof(Item) * length;   
        };

        if(T::is_dict(container)){
            dict_t d = T::to_dict(container);
            fact_infos.reserve(d.size());
            fact_map.reserve(d.size());
            for (auto [key, val] : d) {
                std::string_view fact_id = T::to_string_view(key);
                make_fact_info(fact_id, val);
                index++;
            }
        }else if(T::is_list(container)) {
            list_t lst = T::to_list(container);
            fact_infos.reserve(lst.size());
            fact_map.reserve(lst.size());
            for (auto val : lst) {
                std::string fact_id = std::to_string(index);
                make_fact_info(fact_id, val);
                index++;
            }
        }
        builder = FactSetBuilder(fact_infos.size(), byte_offset);
        // return std::make_tuple(fact_infos, fact_map, byte_offset);
    }

    Item _resolve_possible_fact_ref(Item item, CRE_Type* mbr_type){
    if(item.t_id == T_ID_STR){
        std::string_view item_str = item.as_string();

        // Reference to another fact
        size_t ref_len = ref_prefix.size();
        std::string_view ref_str = {};

        // If the prefix is present like "@bob"
        if(item_str.length() > 0 && 
           !item_str.compare(0, ref_len, ref_prefix)
           ){
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
            auto itr = fact_map.find(ref_str);
            if(itr == fact_map.end()){
                throw std::invalid_argument(
                    "Cannot resolve identifier \"" + std::string(item_str) + "\" for Fact reference." 
                );
            }
            size_t index = itr->second;
            size_t offset = std::get<3>(fact_infos[index]);
            Fact* fact_ptr = (Fact*)(builder.alloc_buffer->data + offset);
            item = Item(fact_ptr);
            }
        }
        return item;
    }

    ref<FactSet> _to_factset(container_t container){
        this->_collect_fact_infos(container);
        
        for(auto& fact_info : fact_infos){
            auto& fact_obj = std::get<0>(fact_info);
            FactType* type = std::get<1>(fact_info);
            size_t length = std::get<2>(fact_info);
            // size_t offset = std::get<3>(fact_info);

            ref<Fact> fact = builder.add_empty(length, type);
            fact->type = type;

            if(T::is_dict(fact_obj)){
                dict_t fact_dict = T::to_dict(fact_obj);
                for (auto [key, val] : fact_dict){

                    std::string_view key_str = T::to_string_view(key);
                    // Ignore the 'type' member (handled above)
                    if(key_str == type_attr){
                        continue;
                    }

                    // If the key is a number then used it as the index
                    char* p;
                    int64_t index = strtol(key_str.data(),&p,10);
                    if(*p != 0) index = -1;

                    // Otherwise get the key's index from the fact type
                    if(type != nullptr){
                        index = type->get_attr_index(key_str);
                    }

                    // Throw error if member index cannot be resolved
                    if(index == -1){
                        std::string type_str = type != nullptr ? std::string(type->name) : "NULL";
                        std::string error_msg = "Key '" + std::string(key_str) + "' is not an integer or named member of fact type '" + type_str + "'.";
                        throw std::runtime_error(error_msg);
                    }

                    Item item = T::to_item(val);
                    item = this->_resolve_possible_fact_ref(item,
                        index == -1 ? nullptr : type->members[index].type
                    );
                    
                    fact->set(index, item);
                }
                // _init_fact(fact, length, type);
                // fact->alloc_buffer = builder.alloc_buffer;
                builder.fact_set->_declare_back(fact);
            }
            // builder.alloc_buffer->add_ref(fact_infos.size());
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

#endif /* _CRE_FactSet_H_ */ 
