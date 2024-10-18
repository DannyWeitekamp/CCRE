#include <vector>
#include <bit>
#include <cstdint>
#include <sstream> 
#include <cstring>
#include <iostream>
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/item.h"


Item* Fact::get(uint32_t a_id){
    Item* data_ptr = std::bit_cast<Item*>(
        std::bit_cast<uint64_t>(this) + sizeof(Fact)
    );
    return &data_ptr[a_id];
}


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

extern "C" void _init_fact(Fact* fact, uint32_t _length, FactType* type){
	fact->type = type;
	fact->f_id = 0;
	fact->hash = 0;
	fact->length = _length;
	fact->parent = (FactSet*) nullptr;
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
	uint32_t length;

	uint32_t n_mbrs = 0;
	if(type == nullptr){
		length = _length;
	}else{
		type->ensure_finalized();
		n_mbrs = type->members.size(); 
		length = std::max(type->members.size(), _length);
	}

	Fact* fact = _alloc_fact(length);
	_init_fact(fact, uint32_t(length), type);

	int i=0;
	for(; i < _length; i++){
		fact->set(i, items[i]);
	}
	_zfill_fact(fact, _length, length);

	
	
	// memcpy(((uint8_t*)fact) + sizeof(Fact), (uint8_t*) items, length * sizeof(Item));
	return fact;
}

Fact* new_fact(FactType* type, const std::vector<Item>& items){
	return new_fact(type, items.data(), items.size());
}


std::vector<Item*> Fact::get_items(){
  std::vector<Item*> out;
  out.reserve(this->length);
  for(int i=0; i < this->length; i++){
    Item* item = this->get(i);
    out.push_back(item);
  }
  return out;
}


int get_unique_id_index(FactType* type){
	if(type != nullptr){
		for(int i=0; i < type->members.size(); i++){

			MemberSpec* mbr_spec = &type->members[i];
			// cout << "TRY: " << i << " " << mbr_spec->builtin_flags << endl;
			if(mbr_spec->get_flag(BIFLG_UNIQUE_ID)){
				return i;
			}
		}
	}
	return -1;
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

string Fact::to_string(){
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



uint64_t CREHash::operator()(Fact* x) {
    uint64_t constexpr fnv_prime = 1099511628211ULL;
    uint64_t constexpr fnv_offset_basis = 14695981039346656037ULL;

    if(x->hash != 0){
        return x->hash;
    }

    uint64_t hash = fnv_offset_basis ^ (x->length * fnv_prime);
    for(int i=0; i < x->length; i++){
        Item item = *x->get(i);
        uint64_t item_hash = CREHash{}(item);
        hash ^= item_hash ^ (i * fnv_prime);
    }

    return hash;
}
