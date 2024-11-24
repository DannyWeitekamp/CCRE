#include <vector>
#include <bit>
#include <cstdint>
#include <sstream> 
#include <cstring>
#include <iostream>
#include <algorithm>
#include "../include/helpers.h"
#include "../include/alloc_buffer.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/item.h"
#include "../include/var.h"



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


// extern "C" Fact* _alloc_fact(uint32_t _length){
	
// 	return ptr;
// }

extern "C" void _zfill_fact(Fact* fact, uint32_t start_a_id, uint32_t end_a_id){
	// Pointer to the 0th Item of the FactHeader
	Item* data_ptr = std::bit_cast<Item*>(
      std::bit_cast<uint64_t>(fact) + sizeof(Fact)
  );
	FactType* type = fact->type;
	// cout << "ZFILL: " << start_a_id << ", " << end_a_id << endl;
  for(int a_id = start_a_id; a_id < end_a_id; a_id++){
	    data_ptr[a_id] = Item();

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

void Fact_dtor(const CRE_Obj* x){
		// cout << "dtor f_id=" << ((Fact*) x)->f_id << ", " << uint64_t(((Fact*) x)->alloc_buffer) << endl;
		// cout << "dtor " << uint64_t(x) << endl;
		Fact* fact = (Fact*) x;
		for(size_t i=0; i < fact->length; i++){
			Item* item = fact->get(i);
			// cout << "~ITEM:" << *item << ", t_id=" << item->t_id << ", borrows=" << uint64_t(item->borrows) << endl;
			if(item->t_id > T_ID_STR && item->borrows){
				CRE_Obj* item_obj = (CRE_Obj*) item->val;
				// cout << "~ITEM REFS: " << item_obj->get_refcount() << endl;
				item_obj->dec_ref();
			}
		}

		if(x->alloc_buffer == nullptr){
	    	free((void*) x);
		}else{
			// NOTE: We need to do this because cannot
			//  write alloc_buffer as a ref<AllocBuffer> 
			// cout << "alloc buff refcount=" << x->alloc_buffer->get_refcount() << endl;
			x->alloc_buffer->dec_ref();
		}
}

Fact::Fact(uint32_t _length, FactType* _type, bool _immutable) :
	CRE_Obj(&Fact_dtor),
	type(_type),
	parent(nullptr),
	f_id(-1),
	length(_type ? std::max(_length, uint32_t(_type->members.size())) : _length),
	hash(0),
	immutable(_immutable)
{}


ref<Fact> empty_fact(FactType* type){
	uint32_t _length = (uint32_t) type->members.size();
	Fact* fact = _alloc_fact(_length);
	_init_fact(fact, _length, type);
	_zfill_fact(fact, 0, _length);
	return fact;
}

ref<Fact> empty_untyped_fact(uint32_t _length){
	Fact* fact = _alloc_fact(_length);
	_init_fact(fact, _length, nullptr);
	_zfill_fact(fact, 0, _length);
	return fact;
}

// void Fact::operator delete(void* p){
// 	CRE_Obj* x = (CRE_Obj*) p;
// 	if(x->alloc_buffer == nullptr){
//     	free(p);
// 	}else{
// 		// NOTE: We need to do this because cannot
// 		//  write alloc_buffer as a ref<AllocBuffer> 
// 		CRE_decref((CRE_Obj*) x->alloc_buffer);
// 	}
// }

// ---------------------
// : new_fact()



inline Fact* _new_fact(Fact* fact, FactType* type, const Item* items, size_t n_items, uint32_t length){

	fact = new (fact) Fact(length, type);

	// Set items
	try{
		for(int i=0; i < n_items; i++){
			fact->set(i, items[i]);
		}
	} catch (const std::exception& e) {
		// Make sure that fact is freed on error
		Fact_dtor(fact);
		throw;
	}
	

	// Zero-fill any trailing items
	_zfill_fact(fact, n_items, length);
	return fact;
}

ref<Fact> new_fact(Fact* fact, FactType* type, const Item* items, size_t n_items){
	uint32_t length = _resolve_fact_len(n_items, type);	
	return _new_fact(fact, type, items, n_items, length);
}

ref<Fact> new_fact(FactType* type, const Item* items, size_t n_items){
	uint32_t length = _resolve_fact_len(n_items, type);	
	Fact* fact = _alloc_fact(length);
	return _new_fact(fact, type, items, n_items, length);
}

ref<Fact> new_fact(Fact* fact, FactType* type, const std::vector<Item>& items){
	uint32_t length = _resolve_fact_len(items.size(), type);	
	return _new_fact(fact, type, items.data(), items.size(), length);
}

ref<Fact> new_fact(FactType* type, const std::vector<Item>& items){
	uint32_t length = _resolve_fact_len(items.size(), type);	
	Fact* fact = _alloc_fact(length);
	return _new_fact(fact, type, items.data(), items.size(), length);
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

// inline size_t Fact::size() const{
// 	return this->length;
// }

void _copy_fact_slice(Fact* src, Fact* dest, size_t start, size_t end){
	size_t length = end-start;

	_init_fact(dest, length, nullptr);	
	size_t src_end = std::min(size_t(src->length), end);

	int j=0;
	int i=start;
	for(; i < src_end; i++, j++){
		Item* item = src->get(i);
		
		if(item->t_id == T_ID_FACT && item->val != 0){
			Fact* item_fact = item->as_fact();
			if(item_fact->immutable){
				// TODO
			}
		}else{
			// Everything else can just be copied;
			dest->set_unsafe(j, *item);
		}
		if(!item->is_primitive() && item->borrows){
			((CRE_Obj*) item->val)->inc_ref();
		}
	}
	if(i < end){
		_zfill_fact(dest, j, end-start);
	}

	dest->immutable = src->immutable;

}

std::tuple<size_t, size_t> Fact::_format_slice(int _start, int _end){
	size_t start = _start >=0 ? _start : this->length + _start;
	size_t end = _end >=0 ? _end : this->length + _end;
		
	int length = end-start;
	if(length < 0 
		//|| length > this->length // (Okay if over)
		){
		throw std::runtime_error(
			"Bad slice: ["+std::to_string(start)+","+std::to_string(end)+"] " +
			"for fact " + this->to_string() + " with length=" + std::to_string(this->length));
	}
	return std::make_tuple(start,end);
}


ref<Fact> Fact::slice_into(Fact* new_fact, int _start, int _end, bool deep_copy){
	
	// cout << "<< start: " << uint64_t(start) <<
	// 				"<< end: " << uint64_t(end) <<
	// 	      "<< LENGTH: " << uint64_t(length) << endl;
	auto [start, end] = _format_slice(_start, _end);
	_copy_fact_slice(this, new_fact, start, end);
	// cout << "ALLOC ADDR: " << uint64_t(new_fact) << endl;
	
	return new_fact;
}

ref<Fact> Fact::slice_into(AllocBuffer& buffer, int _start, int _end, bool deep_copy){
	auto [start, end] = _format_slice(_start, _end);
	size_t length = end-start;
	bool did_malloc = false;
	Fact* new_fact = (Fact *) buffer.alloc_bytes(SIZEOF_FACT(length), did_malloc);
	_copy_fact_slice(this, new_fact, start, end);
	if(!did_malloc){
		new_fact->alloc_buffer = &buffer;
		buffer.inc_ref();
	}
	return new_fact;
}

ref<Fact> Fact::slice(int _start, int _end, bool deep_copy){
	auto [start, end] = _format_slice(_start, _end);
	size_t length = end-start;
	Fact* new_fact = (Fact *) malloc(SIZEOF_FACT(length));
	_copy_fact_slice(this, new_fact, start, end);
	return new_fact;
}

ref<Fact> Fact::copy_into(AllocBuffer& buffer, bool deep_copy){
	bool did_malloc = false;
	Fact* new_fact = (Fact *) buffer.alloc_bytes(SIZEOF_FACT(this->length), did_malloc);
	_copy_fact_slice(this, new_fact, 0, this->length);
	if(!did_malloc){
		new_fact->alloc_buffer = &buffer;
		buffer.inc_ref();
	}
	return new_fact;
}

ref<Fact> Fact::copy(bool deep_copy){
	// Fact* fact = () _alloc_fact(_length);
	// AllocBuffer buffer = AllocBuffer(SIZEOF_FACT(this->length), true);
	// cout << "MALLOC" << SIZEOF_FACT(this->length) << endl; 
	Fact* new_fact = (Fact *) malloc(SIZEOF_FACT(this->length));
	_copy_fact_slice(this, new_fact, 0, this->length);
	// Fact* new_fact = this->slice_into(buffer, 0, this->length, deep_copy);
	return new_fact;
}

std::string Fact::get_unique_id(){
	// std::stringstream ss;
	// FactType* type = fact->type;
	int unique_id_index = get_unique_id_index(type);
	// cout << "unique_id_index: " << unique_id_index << uint64_t(type) << endl ;
	if(unique_id_index != -1){
		// cout << "unique_id_index: " << unique_id_index << endl ;
		Item item = *get(unique_id_index);
		if(item.t_id == T_ID_STR){
			UnicodeItem uitem = std::bit_cast<UnicodeItem>(item);
			return std::string(uitem.data, uitem.length);
		}else{
			return item_to_string(item);
		}	
	}
	return "";
}


std::string Fact::to_string(uint8_t verbosity){
  // FactType* type = fact->type;
  if(verbosity <= 1 && type != nullptr){
  	auto unq_id = get_unique_id();
  	if(!unq_id.empty()){
  		return std::string(unq_id);
  	}
	} // Print more verbosely if no unique_id_index

	std::stringstream ss;
	if(type != nullptr){
		ss << type->name << "(";  
	}else if(immutable){
		ss << "(";  
	}else{
		ss << "Fact(";  
	}
	
	size_t L = size();
	// std::vector<std::string> mbr_strs = {};
	// mbr_strs.reserve(L);
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
		Item* item = get(i);
		ss << item_to_string(*item);
		if(i != L-1){
			ss << ", ";  
		}    
	}
	ss << ")";  
  
	return ss.str();
}


extern "C" void fact_dtor(Fact* fact){
	for(size_t i=0; i < fact->length; i++){
		Item* item = fact->get(i);
		if(item->t_id == T_ID_OBJ){
			ObjItem* o_item = (ObjItem*) item;
			((CRE_Obj*) o_item->data)->dec_ref();
			// CRE_decref((CRE_Obj*) o_item->data);	
		}
	}
	if(fact->alloc_buffer != (void *) NULL){
		fact->alloc_buffer->dec_ref();
		// CRE_decref((CRE_Obj*) fact->alloc_buffer);
	}
	free(fact);
}


std::ostream& operator<<(std::ostream& out, Fact* fact){
	return out << fact->to_string();
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


// extern "C" Item fact_to_item(Fact* fact) {
//     Item item;
//     item.val = std::bit_cast<uint64_t>(fact);
//     uint8_t immutable = fact->immutable;
//     if(immutable){
//     	  item.hash = fact->hash;	
//     }else{
//     	  item.hash = 0;
//     }
//     item.t_id = T_ID_FACT;
//     // cout << "FACT TO ITEM:" << fact << endl;
//     return item;
// }

// Item to_item(Fact* arg) { return fact_to_item(arg);}



// -------------------------------------------------------
// : Fact Hashing 

uint64_t _hash_fact_range(const Fact* x, uint16_t start, uint16_t end){
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
    return _hash_fact_range((const Fact*) x.fact, x.start, x.end_);

    
}

// -------------------------------------------------------
// : Fact Equals 


bool item_eq(const Item& ia, const Item& ib){
	// cout << "ITEM A: " << ia << " ITEM B: " << ib << endl;
	// cout << "HASH A: " << CREHash{}(ia) << " HASH B: " << CREHash{}(ib) << endl;
	if(ia.t_id != ib.t_id) return false;
	switch (ia.t_id) {
		case T_ID_FLOAT:
			{
				double da = ia.as_float();
				double db = ib.as_float();
				if(da != db) return false;
				break;
			}
		case T_ID_FACT:
			if(*ia.as_fact() != *ib.as_fact()) return false;
			break;
		case T_ID_VAR:
			if(*ia.as_var() != *ib.as_var()) return false;
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
	const Fact* fact_a = (const Fact*) this->fact;
	const Fact* fact_b = (const Fact*) other.fact;
	for(size_t i=0; i < this->size(); i++){
		Item* ia = fact_a->get(this->start + i);
		Item* ib = fact_b->get(other.start + i);
		if(!item_eq(*ia, *ib)) return false;
	}
	return true;
}




