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

using std::cout;
using std::endl;


// Forward Declaration
struct FactChange;

//--------------------------------------------------------------
// : FactSet

class FactSet : public CRE_Obj {

public:
	// -- Members --
	std::vector<Fact*> facts;
	std::vector<uint32_t> empty_f_ids;
	uint64_t _size;
    std::vector<FactChange> change_queue;

	// -- Methods -- 
	FactSet(size_t n_facts=0);
	FactSet(std::vector<Fact*> facts);
    // ~FactSet();

    inline size_t size() const {return _size;}
	uint32_t declare(Fact* fact);
	void retract(uint32_t f_id);
	void retract(Fact* fact);
    std::vector<Fact*> get_facts();

    Fact* add_fact(FactType* type, const Item* items, size_t n_items);
    Fact* add_fact(FactType* type, const std::vector<Item>& items);

    std::string to_string(
        std::string_view format="FactSet{{\n  {}\n}}",
        std::string_view delim="\n  "
    );

    static FactSet* from_json(char* json_str, size_t length=-1, bool copy_buffer=true);
    static FactSet* from_json_file(const char* json);
    static char* to_json(FactSet* fs);


    // std::string to_string();

    inline void _declare_back(Fact* fact){

        // Equivalent of declare(fs, fact) but don't bother with empties
        // _init_fact(fact, length, type);
        uint32_t f_id = (uint32_t) facts.size();
        fact->f_id = f_id;
        fact->parent = this;
        facts.push_back(fact);
        _size++;   
    }

// -- Iterator --
    class Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Fact*;
        using pointer           = Fact**;
        using reference         = Fact*&;

    private:
        std::vector<Fact*>::iterator current;
        std::vector<Fact*>::iterator end;

    public:
        Iterator(std::vector<Fact*>::iterator s,
                 std::vector<Fact*>::iterator e){
            current = s;
            end = e;
        }

        reference operator*() const { return *current; }
        pointer operator->() const { return &*current; }
        Iterator& operator++() { 
            do{
                current++;
            }while(current != end and 
                 *current == (Fact*) NULL);
            
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
    AllocBuffer* alloc_buffer;
    FactSet* fact_set;

    // --- Methods ---
    FactSetBuilder(size_t size=0, size_t buffer_size=0);

    inline Fact* next_empty(size_t size){
        Fact* fact = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size));
        fact = new (fact) Fact(size, nullptr, false);
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
    inline Fact* add_fact(FactType* __restrict  type,
                          const Item* __restrict items,
                          uint32_t n_items, bool immutable=false){

        uint32_t size =_resolve_fact_len(n_items, type);
        Fact* fact = (Fact*) alloc_buffer->alloc_bytes(SIZEOF_FACT(size));
        fact = new (fact) Fact(size, type, immutable);

        memcpy(((uint8_t*)fact) + sizeof(Fact), (uint8_t*) items, size * sizeof(Item));
        fact_set->_declare_back(fact);
        return fact;
    }

    inline Fact* add_fact(FactType* __restrict  type,
                          const std::vector<Item>& items,
                          bool immutable=false){
        return add_fact(type, items.data(), items.size(), immutable);
    }
};

// extern "C" Fact* FactSetBuilder_add_fact(
//         FactSetBuilder* fs,
//         FactType* type, const Item* items, uint32_t _length);

// inline void _init_fact(Fact* fact, uint32_t _length, FactType* type){
//     fact->type = type;
//     fact->f_id = 0;
//     fact->hash = 0;
//     fact->length = _length;
//     fact->parent = (FactSet*) nullptr;
// }


// void _declare_from_empty(const FactSetBuilder& fsb, Fact* fact, uint32_t length, FactType* type);

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
