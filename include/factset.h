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

class FactSet : public CRE_Obj {

public:
	// -- Members --
	vector<Fact*> facts;
	vector<uint32_t> empty_f_ids;
	uint64_t size;


	// -- Methods -- 
	FactSet(size_t n_facts=0);
	FactSet(vector<Fact*> facts);

	uint32_t declare(Fact* fact);
	void retract(uint32_t f_id);
	void retract(Fact* fact);
    vector<Fact*> get_facts();

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

// stringstream fs_to_stream(const FactSet* fs, string delim = ", ");
extern "C" string fs_to_string(FactSet* fs, const string& delim = ", ");

#endif /* _CRE_FactSet_H_ */ 
