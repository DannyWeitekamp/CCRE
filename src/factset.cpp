

#include <string>
#include <vector>
#include <bit>
#include <cstdint>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <sstream> 
#include <cstring>

// #define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <fmt/ranges.h>

#include "../include/item.h"
#include "../include/types.h"
#include "../include/fact.h"
#include "../include/factset.h"

using namespace std;



// ---- Main C interface ----

// extern "C" bool new_fact_set(size_t n_facts){
// 	//
// }

extern "C" bool is_declared(Fact* fact){
	return fact->parent != (FactSet*) NULL;
}

extern "C" void assert_undeclared(FactSet* fs, Fact* fact){
	if(is_declared(fact)){
		stringstream err;
		if(fact->parent == fs){
			err << "Redeclaration of Fact: " << fact->to_string();
		}else{
			err << "Fact declared in other FactSet: " << fact->to_string()
				  << ". Copy first fact.copy().";
		}
		throw std::runtime_error(err.str());
	}	
}

extern "C" uint32_t declare(FactSet* fs, Fact* fact){
	assert_undeclared(fs, fact);
	

	uint32_t f_id;
	if(fs->empty_f_ids.size() > 0){
		f_id = fs->empty_f_ids.back();
		fs->empty_f_ids.pop_back();
		fs->facts[f_id] = fact;
		// cout << "Insert" << endl;
	}else{
		f_id = (uint32_t) fs->facts.size();	
		fs->facts.push_back(fact);
		// cout << "Pushback" <<  endl;
	}
	fact->f_id = f_id;
	fact->parent = fs;
	CRE_incref(fact);

	fs->size++;

	return f_id;
}

extern "C" void retract_f_id(FactSet* fs, uint32_t f_id){
	if(f_id >= fs->facts.size()){
		stringstream err;
		err << "Retract f_id=" << f_id << " is out of range.";
		throw std::out_of_range(err.str());
	}
	Fact* fact = fs->facts[f_id];

	if(fact == (Fact*) NULL){
		stringstream err;
		err << "Attempt to retract retracted f_id.";
		throw std::runtime_error(err.str());
	}
	
	fs->facts[f_id] = NULL;
	fs->empty_f_ids.push_back(f_id);
	fs->size--;

	fact->f_id = 0;
	fact->parent = (FactSet*) NULL;
	CRE_decref(fact);
}

extern "C" void retract(FactSet* fs, Fact* fact){
	if(fact->parent != fs){
		stringstream err;
		err << "Attempt to retract ";
		if(fact->parent == (FactSet *) NULL){
			err << "undeclared fact:";
		}else{
			err << "fact declared to other FactSet:";
		}
		err << fact->to_string();
		throw std::runtime_error(err.str());
	}

	uint32_t f_id = fact->f_id;
	retract_f_id(fs, f_id);
}

extern "C" vector<Fact*> fs_get_facts(FactSet* fs){
	vector<Fact*> facts = {};
	facts.reserve(fs->size);
	for (auto it = fs->begin(); it != fs->end(); ++it) { 
		Fact* fact = (*it);
		facts.push_back(fact);
    }
    return facts;
}

extern "C" string fs_to_string(FactSet* fs, const string& delim){
	vector<string> fact_strs = {};
	fact_strs.reserve(fs->size);
	for (auto it = fs->begin(); it != fs->end(); ++it) {
		Fact* fact = (*it);
		fact_strs.emplace_back(fact_to_string(fact));
	}
	return fmt::format("FactSet{{ {} }}", fmt::join(fact_strs, delim));	
}


// extern "C" string 	// cout << "START" << endl;
	// auto it = fs->begin();
	// int i = 0;
	// while(true){
	// 	cout << "LOOP " << i << endl;
	// 	Fact* fact = *it;
	// 	s << fact_to_string(fact);
	// 	++it;
	// 	i++;
	// 	if(it == fs->end()){
	// 		break;
	// 	}
	// 	s << delim;
		
	// }
	// s << " }";
	// cout << "FACT SET TO STRING" << endl;

	// return s.str();

	// cout << "START" << endl;
	// auto it = fs->begin();
	// int i = 0;
	// while(true){
	// 	cout << "LOOP " << i << endl;
	// 	Fact* fact = *it;
	// 	s << fact_to_string(fact);
	// 	++it;
	// 	i++;
	// 	if(it == fs->end()){
	// 		break;
	// 	}
	// 	s << delim;
		
	// }
	// s << " }";
	// cout << "FACT SET TO STRING" << endl;

	// return s.str();



// ---- Method Declarations ----

FactSet::FactSet(size_t n_facts){
	this->facts = {};
	this->empty_f_ids = {};
	this->facts.reserve(n_facts);
}

uint32_t FactSet::declare(Fact* fact){
	return ::declare(this, fact);
}

void FactSet::retract(uint32_t f_id){
	return ::retract_f_id(this, f_id);
}

void FactSet::retract(Fact* fact){
	return ::retract(this, fact);	
}

ostream& operator<<(std::ostream& out, FactSet* fs){
	return out << fs_to_string(fs);
}
