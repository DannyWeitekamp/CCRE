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

using namespace std;

//--------------------------------------------------------------
// : FactSet

class FactSet : public CRE_Obj {

public:
	// -- Members --
	vector<Fact*> facts;
	vector<uint32_t> empty_f_ids;
	uint64_t size;


	// -- Methods -- 
	FactSet(size_t n_facts=0);
	FactSet(vector<Fact*> facts);
    // ~FactSet();

	uint32_t declare(Fact* fact);
	void retract(uint32_t f_id);
	void retract(Fact* fact);
    vector<Fact*> get_facts();
    Fact* add_fact(FactType* type, const std::vector<Item>& items);

// -- Iterator --
    class Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Fact*;
        using pointer           = Fact**;
        using reference         = Fact*&;

    private:
        vector<Fact*>::iterator current;
        vector<Fact*>::iterator end;

    public:
        Iterator(vector<Fact*>::iterator s,
                 vector<Fact*>::iterator e){
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

ostream& operator<<(std::ostream& out, FactSet* fs);

extern "C" bool is_declared(Fact* fact);
extern "C" void assert_undeclared(FactSet* fs, Fact* fact);
extern "C" uint32_t declare(FactSet* fs, Fact* fact);
extern "C" void retract_f_id(FactSet* fs, uint32_t f_id);
extern "C" void retract(FactSet* fs, Fact* fact);
extern "C" void fs_dtor(FactSet* fs);
extern "C" FactSet* FactSet_from_json(char* json_str, size_t length=-1, bool copy_buffer=true);
extern "C" FactSet* FactSet_from_json_file(const char* json);
extern "C" char* FactSet_to_json(FactSet*);
extern "C" string FactSet_to_string(FactSet* fs, const string& delim = ", ");


//--------------------------------------------------------------
// : AllocBuffer

class AllocBuffer : public CRE_Obj {
public:
    // --- Members ---
    uint8_t* data;
    uint8_t* head;
    uint8_t* end;
    size_t alignment;
    
    // --- Methods ---
    AllocBuffer(size_t n_bytes);
    void add_data(uint8_t* data, size_t size);
};


//--------------------------------------------------------------
// : FactSetBuilder

class FactSetBuilder : public CRE_Obj {
public:
    // --- Members ---
    AllocBuffer* alloc_buffer;
    FactSet* fact_set;

    // --- Methods ---
    FactSetBuilder(size_t size, size_t buffer_size);

    Fact* next_empty(size_t size);
    Fact* add_fact(FactType* type, const std::vector<Item>& items);
};

extern "C" Fact* FactSetBuilder_add_fact(
        FactSetBuilder* fs,
        FactType* type, const Item* items, uint32_t _length);


void _declare_from_empty(FactSetBuilder* fsb, Fact* fact, uint32_t length, FactType* type);

#endif /* _CRE_FactSet_H_ */ 
