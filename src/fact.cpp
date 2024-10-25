#include <vector>
#include <bit>
#include <cstdint>
#include <sstream> 
#include <cstring>
#include <iostream>
#include <algorithm>
#include "../include/alloc_buffer.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/item.h"



// Item* Fact::get(uint32_t a_id) const{
//     Item* data_ptr = std::bit_cast<Item*>(
//         std::bit_cast<uint64_t>(this) + sizeof(Fact)
//     );
//     return &data_ptr[a_id];
// }

// void Fact::set_unsafe(uint32_t a_id, const Item& val){
//     Item* data_ptr = std::bit_cast<Item*>(
//         std::bit_cast<uint64_t>(this) + sizeof(Fact)
//     );
//     data_ptr[a_id] = val;
// }


extern "C" Fact* _alloc_fact(uint32_t _length){
	Fact* ptr = (Fact*) malloc(sizeof(Fact) + _length * sizeof(Item));
	return ptr;
}

extern "C" void _zfill_fact(Fact* fact, uint32_t start_a_id, uint32_t end_a_id){
	// Pointer to the 0th Item of the FactHeader
	Item* data_ptr = std::bit_cast<Item*>(
      std::bit_cast<uint64_t>(fact) + sizeof(Fact)
  );
	FactType* type = fact->type;
	// cout << "ZFILL: " << start_a_id << ", " << end_a_id << endl;
  for(int a_id = start_a_id; a_id < end_a_id; a_id++){
	    data_ptr[a_id] = empty_item();

	    if(type != nullptr && a_id < type->members.size()){
	    	CRE_Type* mbr_typ = (&type->members[a_id])->type;
	    	// cout << uint64_t(mbr_typ) << " FILL BACK" << std::to_string(a_id) <<
	    	//  		" T_ID: " << mbr_typ->t_id <<
	    	//  		" Type: " << mbr_typ << " [" << 
	    	//  							int(mbr_typ->kind) << "]" << endl;
	    	data_ptr[a_id].t_id = mbr_typ->t_id;
	    }
	}
}



extern "C" Fact* empty_fact(FactType* type){
	uint32_t _length = (uint32_t) type->members.size();
	Fact* fact = _alloc_fact(_length);
	_init_fact(fact, _length, type);
	_zfill_fact(fact, 0, _length);
	return fact;
}

extern "C" Fact* empty_untyped_fact(uint32_t _length){
	Fact* fact = _alloc_fact(_length);
	_init_fact(fact, _length, nullptr);
	_zfill_fact(fact, 0, _length);
	return fact;
}

extern "C" Fact* new_fact(FactType* type, const Item* items, size_t _length){
	// Resolve length + finalize type
	uint32_t length;
	if(type == nullptr){
		length = _length;
	}else{
		type->ensure_finalized();
		length = std::max(type->members.size(), _length);
	}

	// Initialize fact's head
	Fact* fact = _alloc_fact(length);
	_init_fact(fact, uint32_t(length), type);

	// Set items
	for(int i=0; i < _length; i++){
		fact->set(i, items[i]);
	}

	// Zero-fill any trailing items
	_zfill_fact(fact, _length, length);
	return fact;
}

Fact* new_fact(FactType* type, const std::vector<Item>& items){
	return new_fact(type, items.data(), items.size());
}

std::vector<Item*> Fact::get_items() const{
  std::vector<Item*> out;
  out.reserve(this->length);
  for(int i=0; i < this->length; i++){
    Item* item = this->get(i);
    out.push_back(item);
  }
  return out;
}

inline size_t Fact::size() const{
	return this->length;
}

Fact* Fact::slice_into(AllocBuffer& buffer, int start, int end, bool deep_copy){
	start = start >=0 ? start : this->length + start;
	end = end >=0 ? end : this->length + end;
		
	int length = end-start;
	if(length < 0 || length > this->length){
		throw std::runtime_error(
			"Bad slice: ["+std::to_string(start)+","+std::to_string(end)+"] " +
			"for fact " + fact_to_string(this) + " with length=" + std::to_string(this->length));
	}
	// size_t fact_n_bytes = SIZEOF_FACT(length);

	// if(buffer->head + fact_n_bytes >= buffer->end){
	// 	buffer->resize(buffer->size + std::min(buffer->size, size_t(4096)));
	// }
	// Init Fact Header
	// cout << "<< start: " << uint64_t(start) <<
	// 				"<< end: " << uint64_t(end) <<
	// 	      "<< LENGTH: " << uint64_t(length) << endl;
	Fact* new_fact = (Fact *) buffer.alloc_bytes(SIZEOF_FACT(length));
	// cout << "ALLOC ADDR: " << uint64_t(new_fact) << endl;
	_init_fact(new_fact, length, nullptr);
	new_fact->immutable = this->immutable;

	for(int i=start; i < end; i++){
		Item* item = this->get(i);
		
		if(item->t_id == T_ID_FACT && item->val != 0){
			Fact* fact_item = item_get_fact(item);
			if(fact_item->immutable){

			}
		}else{
			// Everything else can just be copied;
			new_fact->set_unsafe(i, *item);
		}
	}
	return new_fact;
}

Fact* Fact::slice(int start, int end, bool deep_copy){
	// Fact* fact = () _alloc_fact(_length);
	AllocBuffer buffer = AllocBuffer(SIZEOF_FACT(this->length), true);
	Fact* new_fact = this->slice_into(buffer, start, end, deep_copy);
	return new_fact;
}

Fact* Fact::copy_into(AllocBuffer& buffer, bool deep_copy){
	Fact* new_fact = this->slice_into(buffer, 0, this->length, deep_copy);
	return new_fact;
}

Fact* Fact::copy(bool deep_copy){
	// Fact* fact = () _alloc_fact(_length);
	AllocBuffer buffer = AllocBuffer(SIZEOF_FACT(this->length), true);
	Fact* new_fact = this->slice_into(buffer, 0, this->length, deep_copy);
	return new_fact;
}

extern "C" std::string fact_to_unique_id(Fact* fact){
	std::stringstream ss;
	FactType* type = fact->type;
	int unique_id_index = get_unique_id_index(type);
	// cout << "unique_id_index: " << unique_id_index << uint64_t(type) << endl ;
	if(unique_id_index != -1){
		// cout << "unique_id_index: " << unique_id_index << endl ;
		Item item = *fact->get(unique_id_index);
		if(item.t_id == T_ID_STR){
			UnicodeItem uitem = std::bit_cast<UnicodeItem>(item);
			ss << std::string(uitem.data, uitem.length);
		}else{
			ss << item_to_string(item);
		}	
		return ss.str();
	}
	return "";
}


extern "C" std::string fact_to_string(Fact* fact, uint8_t verbosity){
  FactType* type = fact->type;
  if(verbosity <= 1 && type != nullptr){
  	auto unq_id = fact_to_unique_id(fact);
  	if(!unq_id.empty()){
  		return std::string(unq_id);
  	}
	} // Print more verbosely if no unique_id_index

	std::stringstream ss;
	if(type != nullptr){
		ss << type->name << "(";  
	}else{
		ss << "Fact(";  
	}
	
	size_t L = fact->length;
	for(int i=0; i < L; i++){
		if(type != nullptr && i < type->members.size()){
			MemberSpec* mbr_spec = &type->members[i];
			if(mbr_spec->get_flag(BIFLG_VERBOSITY) >= verbosity){
				continue;
			}

			if(mbr_spec->name.length() > 0){
				ss << mbr_spec->name << "=";
			}
		}
		Item* item = fact->get(i);
		ss << item_to_string(*item);
		if(i != L-1){
			ss << ", ";  
		}    
	}
	ss << ")";  
  
	return ss.str();
}

std::string Fact::to_string(){
	return fact_to_string(this);
}

extern "C" void fact_dtor(Fact* fact){
	for(size_t i=0; i < fact->length; i++){
		Item* item = fact->get(i);
		if(item->t_id == T_ID_OBJ){
			ObjItem* o_item = (ObjItem*) item;
			CRE_decref((CRE_Obj*) o_item->data);	
		}
	}
	if(fact->alloc_buffer != (void *) NULL){
		CRE_decref((CRE_Obj*) fact->alloc_buffer);
	}
	free(fact);
}


std::ostream& operator<<(std::ostream& out, Fact* fact){
	return out << fact_to_string(fact);
}



// Fact::Iterator Fact::begin() const noexcept{
// 	uint8_t* data = (uint8_t*) this;
//   return Iterator((Item*) (data + sizeof(Fact)));
// }

// Fact::Iterator Fact::end() const noexcept{
// 	uint8_t* data = (uint8_t*) this;
//     return Iterator((Item*) (data + sizeof(Fact) + this->length*sizeof(Item)));
// }

// Enable Iteration over Fact* 
Fact::Iterator begin(const Fact* fact){return fact->begin();}
Fact::Iterator end(const Fact* fact){return fact->end();}


extern "C" Item fact_to_item(Fact* fact) {
    Item item;
    item.val = std::bit_cast<uint64_t>(fact);
    uint8_t immutable = fact->immutable;
    if(immutable){
    	  item.hash = fact->hash;	
    }else{
    	  item.hash = 0;
    }
    item.t_id = T_ID_FACT;
    // cout << "FACT TO ITEM:" << fact << endl;
    return item;
}

Item to_item(Fact* arg) { return fact_to_item(arg);}



// -------------------------------------------------------
// : Fact Hashing 

uint64_t _hash_fact_range(Fact* x, uint16_t start, uint16_t end){
	uint64_t constexpr fnv_prime = 1099511628211ULL;
  uint64_t constexpr fnv_offset_basis = 14695981039346656037ULL;


  // cout << "Fact: " << x << endl;
 	uint64_t hash = fnv_offset_basis; //^ (end-start * fnv_prime);
  for(uint16_t i=start; i < end; i++){
  		// cout << "i: " << i << endl;
      Item& item = *x->get(i);
      // cout << "item: " << item << endl;
      uint64_t item_hash = CREHash{}(item);
      hash ^= item_hash ^ (i * fnv_prime);
  }
  return hash; 
}


uint64_t CREHash::operator()(Fact* x) {
    if(x->hash != 0){
        return x->hash;
    }   
    uint64_t hash = _hash_fact_range(x, 0, x->length);
    x->hash = hash;
    return hash;
}

uint64_t CREHash::operator()(const FactView& x) const{
    return _hash_fact_range(x.fact, x.start, x.end_);

    
}

// -------------------------------------------------------
// : Fact Equals 


bool item_eq(const Item& ia, const Item& ib){
	// cout << "ITEM A: " << ia << " ITEM B: " << ib << endl;
	if(ia.t_id != ib.t_id) return false;
	switch (ia.t_id) {
		case T_ID_FLOAT:
			{
				double da = item_get_float(ia);
				double db = item_get_float(ib);
				if(da != db) return false;
				break;
			}
		case T_ID_FACT:
			if(*item_get_fact(ia) != *item_get_fact(ib)) return false;
			break;
		case T_ID_VAR:
			// TODO
			break;
		case T_ID_FUNC:
			// TODO
			break;
		case T_ID_LITERAL:
			// TODO
			break;
		case T_ID_CONDITIONS:
			// TODO
			break;
		// Default case should work for integer types
		//  and interned strings
		default:
			if(ia.val != ib.val) return false;
			break;
	}
	return true;
}

bool Fact::operator==(const Fact& other) const {
	if(length != other.length) return false;

	for(size_t i=0; i < length; i++){
		Item* ia = this->get(i);
		Item* ib = other.get(i);
		if(!item_eq(*ia, *ib)) return false;
	}
	return true;
};

bool FactView::operator==(const FactView& other) const {
	if(this->size() != other.size()) return false;
	Fact* fact_a = this->fact;
	Fact* fact_b = other.fact;
	for(size_t i=0; i < this->size(); i++){
		Item* ia = fact_a->get(this->start + i);
		Item* ib = fact_b->get(other.start + i);
		if(!item_eq(*ia, *ib)) return false;
	}
	return true;
}




