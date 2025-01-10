

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

using std::cout;
using std::endl;



//--------------------------------------------------------------
// : FactSet

// ---- Main C interface ----

bool is_declared(Fact* fact){
	return fact->parent != nullptr;
}

void assert_undeclared(FactSet* fs, Fact* fact){
	if(is_declared(fact)){
		std::stringstream err;
		if(fact->parent == fs){
			err << "Redeclaration of Fact: " << fact->to_string();
		}else{
			err << "Fact declared in other FactSet: " << fact->to_string()
				  << ". Copy first fact.copy().";
		}
		throw std::runtime_error(err.str());
	}	
}

uint32_t declare(FactSet* fs, Fact* fact){
	assert_undeclared(fs, fact);
	
	// cout << "A" << endl;
	// cout << "refcount before:" << fact->get_refcount() << endl;
	uint32_t f_id;
	if(fs->empty_f_ids.size() > 0){
		// cout << "Insert: " << fs->empty_f_ids.size() << endl;
		f_id = fs->empty_f_ids.back();
		// cout << "f_id: " << f_id << endl;
		fs->empty_f_ids.pop_back();
		fs->facts[f_id] = fact;
		
	}else{
		// cout << "Pushback" <<  endl;
		f_id = (uint32_t) fs->facts.size();	
		
		fs->facts.push_back(fact);
		
	}
	// cout << "refcount after:" << fact->get_refcount() << endl;
	// cout << "B" << endl;

	fact->f_id = f_id;
	fact->parent = fs;
	// fact->inc_ref();

	// cout << "C" << endl;
	fs->_size++;

	return f_id;
}

void retract_f_id(FactSet* fs, uint32_t f_id){
	if(f_id >= fs->facts.size()){
		std::stringstream err;
		err << "Retract f_id=" << f_id << " is out of range.";
		throw std::out_of_range(err.str());
	}
	Fact* fact = fs->facts[f_id];

	if(fact == nullptr){
		std::stringstream err;
		err << "Attempt to retract retracted f_id.";
		throw std::runtime_error(err.str());
	}
	
	fs->facts[f_id] = nullptr;
	fs->empty_f_ids.push_back(f_id);
	fs->_size--;

	fact->f_id = 0;
	fact->parent = nullptr;
	// fact->dec_ref();
}

void retract(FactSet* fs, Fact* fact){
	if(fact->parent != fs){
		std::stringstream err;
		err << "Attempt to retract ";
		if(fact->parent == nullptr){
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

std::vector<ref<Fact>> fs_get_facts(FactSet* fs){
	std::vector<ref<Fact>> facts = {};
	facts.reserve(fs->size());
	for (auto it = fs->begin(); it != fs->end(); ++it) { 
		Fact* fact = (*it);
		facts.push_back(fact);
    }
    return facts;
}

std::string FactSet::to_string(
			std::string_view format,
			std::string_view delim){

	std::vector<std::string> fact_strs = {};
	// cout << "size" << fs->size << endl;
	fact_strs.reserve(size());
	for (auto it = begin(); it != end(); ++it) {
		Fact* fact = (*it);
		fact_strs.emplace_back(fact->to_string());
	}
	return fmt::format(fmt::runtime(format), fmt::join(fact_strs, delim));	
}

std::vector<uint8_t> FactSet::long_hash_bytes(size_t byte_width){

	// std::vector<uint8_t>* hash_bytes_ptr = new std::vector<uint8_t>(byte_width, 0);
	std::vector<uint8_t> hash_bytes = std::vector<uint8_t>(byte_width, 0);

	for (auto it = begin(); it != end(); ++it) {
		Fact* fact = (*it);
		uint64_t fact_hash = CREHash{}(fact);

		uint8_t b0 = (fact_hash >> 0)  & 0xFF;  // Least significant byte
		uint8_t b1 = (fact_hash >> 8)  & 0xFF;
		uint8_t b2 = (fact_hash >> 16) & 0xFF;
		uint8_t b3 = (fact_hash >> 24) & 0xFF;
		uint8_t b4 = (fact_hash >> 32) & 0xFF;
		uint8_t b5 = (fact_hash >> 40) & 0xFF;
		uint8_t b6 = (fact_hash >> 48) & 0xFF;
		uint8_t b7 = (fact_hash >> 56) & 0xFF;  // Most significant byte

		hash_bytes[b7 % byte_width] ^= b0;
		hash_bytes[b0 % byte_width] ^= b1;
		hash_bytes[b1 % byte_width] ^= b2;
		hash_bytes[b2 % byte_width] ^= b3;
		hash_bytes[b3 % byte_width] ^= b4;
		hash_bytes[b4 % byte_width] ^= b5;
		hash_bytes[b5 % byte_width] ^= b6;
		hash_bytes[b6 % byte_width] ^= b7;
	}

	return hash_bytes;
}

std::string FactSet::long_hash_string(size_t byte_width){
	std::vector<uint8_t> hash = long_hash_bytes(byte_width);
	return bytes_to_base64(hash);
}

// std::string FactSet::to_string(){
// 	return to_string("FactSet{{\n  {}\n}}", "\n  ");
// }

// extern "C" void fs_dtor(FactSet* fs){
// 	for (auto& fact : fs->facts) {
// 		if(fact == (Fact *) NULL) 
// 			continue;
// 		CRE_decref(fact);
// 	}
// }

// ---- Method Declarations ----

void FactSet_dtor(const CRE_Obj* x){
	// cout << "FactSet_dtor:" << uint64_t(x) << endl;
	// cout << (FactType* ) x << endl; 
	delete ((FactSet*) x);
}

FactSet::FactSet(size_t n_facts) :
	facts({}), empty_f_ids({}), _size(0)
{
	this->facts.reserve(n_facts);
	dtor = &FactSet_dtor;
	// this->facts = {};
	// this->empty_f_ids = {};
	
	// this->dtor = (CRE_dtor_function) &fs_dtor;
	// this->_size = 0;

	// cout << "empty_f_ids:" << this->empty_f_ids.size() << endl;
}

// FactSet::~FactSet(){
// 	for (auto& fact : this->facts) {
// 		if(fact == (Fact *) NULL) 
// 			continue;
// 		CRE_decref(fact);
// 	}
// }

uint32_t FactSet::declare(Fact* fact){
	return ::declare(this, fact);
}

void FactSet::retract(uint32_t f_id){
	return ::retract_f_id(this, f_id);
}

void FactSet::retract(Fact* fact){
	return ::retract(this, fact);	
}

std::ostream& operator<<(std::ostream& out, FactSet* fs){
	return out << fs->to_string("FactSet{{{}}} ", ", ");
}

// ref<Fact> FactSet::declare_new(FactType* type, const Item* items, size_t n_items){
// 	ref<Fact> fact = new_fact(type, items, n_items);
// 	this->declare(fact);
// 	return fact;
// }

// ref<Fact> FactSet::declare_new(FactType* type, const std::vector<Item>& items){
// 	ref<Fact> fact = new_fact(type, items);
// 	this->declare(fact);
// 	return fact;
// }




//--------------------------------------------------------------
// : FactSetBuilder


FactSetBuilder::FactSetBuilder(size_t size, size_t buffer_size) :
	alloc_buffer(new AllocBuffer(buffer_size)),
	fact_set(new FactSet(size)){
}

// extern "C" Fact* FactSetBuilder_add_fact(
// 		FactSetBuilder* fsb,
// 		FactType* type, const Item* items, uint32_t _length){

// 	uint32_t length;

// 	if(type == NULL){
// 		length = _length;
// 	}else{
// 		length = std::max((uint32_t) type->members.size(), _length);
// 	}

// 	Fact* fact = fsb->next_empty(length);

// 	// cout << "L: " << length << "  sizeof ITEM: " << sizeof(Item) << endl ;
// 	memcpy(((uint8_t*)fact) + sizeof(Fact), (uint8_t*) items, length * sizeof(Item));

// 	_declare_to_empty(fsb->fact_set, fact, length, type);

// 	return fact;
// }



// inline Fact* FactSetBuilder::next_empty(size_t size){
// 	Fact* fact;
// 	AllocBuffer& buff = this->alloc_buffer;

// 	uint8_t* next_head = buff.head + sizeof(Fact) + size * sizeof(Item);	

// 	if(next_head <= buff.end){
// 		fact = (Fact*) buff.head;
// 		buff.head = next_head;
// 		// cout << sizeof(Fact) << endl;
// 		// cout << "Buff: " << uint64_t(buff.head) << " " << endl; 
// 	}else{
// 		fact = empty_untyped_fact(size);
// 		// cout << "ALLOCED! " << endl; 
// 	}
// 	fact->length = size;
// 	fact->type = NULL;
// 	return fact;
// }


// Fact* FactSetBuilder::add_fact(FactType* type, const std::vector<Item>& items){
// 	return FactSetBuilder_add_fact(this, type, items.data(), (uint32_t) items.size());
	
// }	








