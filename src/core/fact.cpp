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

namespace cre {

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

void Fact::ensure_unique_id(){
	if(type != nullptr){
	  int16_t uid_ind = type->unique_id_index;
	  if(uid_ind < length){
	    unique_id_index = uid_ind;
	  }
	}

	// NOTE: This is only safe because we always intern strings
	if(unique_id_index != -1){
		const Item& item = get(unique_id_index);
		control_block->unique_id = (char*) item.data;//get_unique_id();	
	}
	
}


void Fact_dtor(const CRE_Obj* x){
		cout << "Fact_dtor: " << uint64_t(x) << endl;
		// cout << "dtor f_id=" << ((Fact*) x)->f_id << ", " << uint64_t(((Fact*) x)->alloc_buffer) << endl;
		// cout << "dtor " << uint64_t(x) << endl;
		Fact* fact = (Fact*) x;

		// cout << "FACT DTOR: " << fact << endl;
		for(size_t i=0; i < fact->length; i++){
			Item* item = fact->get_ptr(i);
			item->release();
			// cout << "~ITEM: " << "i=" << i << " " << item << ", t_id=" << item.get_t_id() << ", borrows=" << uint64_t(item.borrows) << endl;
			// if(!item.is_primitive() && item.borrows){
			// 	// CRE_Obj* item_obj = ;
			// 	cout << "i=" << i << " ~ITEM REFS: " << ((CRE_Obj*) item.val)->get_refcount() << endl;
			// 	cout << item << endl;
			// // 	item_obj->dec_ref();
			// }
		
		}

		if(x->control_block->alloc_buffer == nullptr){
	    	free((void*) x);
		}else{
			// NOTE: We need to do this because cannot
			//  write alloc_buffer as a ref<AllocBuffer> 
			// cout << "alloc buff refcount=" << x->alloc_buffer->get_refcount() << endl;
			x->control_block->alloc_buffer->dec_ref();
		}
}



// ref<Fact> empty_fact(FactType* type){
// 	uint32_t _length = (uint32_t) type->members.size();
// 	Fact* fact = alloc_fact(_length);
// 	_init_fact(fact, _length, type);
// 	_zfill_fact(fact, 0, _length);
// 	return fact;
// }

// ref<Fact> empty_untyped_fact(uint32_t length){
// 	Fact* fact_addr = alloc_fact(length);
// 	ref<Fact> fact = new (fact_addr) Fact(length, nullptr, false);
// 	// _init_fact(fact, _length, nullptr);
// 	_zfill_fact(fact, 0, length);
// 	return fact;
// }

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







// ref<Fact> new_fact(Fact* fact, FactType* type, const Item* items, size_t n_items){
// 	uint32_t length = _resolve_fact_len(n_items, type);	
// 	return _new_fact(fact, type, items, n_items, length);
// }

// ref<Fact> new_fact(FactType* type, const Item* items, size_t n_items){
// 	uint32_t length = _resolve_fact_len(n_items, type);	
// 	Fact* fact = alloc_fact(length);
// 	return _new_fact(fact, type, items, n_items, length);
// }

// ref<Fact> new_fact(Fact* fact, FactType* type, const std::vector<Item>& items){
// 	uint32_t length = _resolve_fact_len(items.size(), type);	
// 	return _new_fact(fact, type, items.data(), items.size(), length);
// }

// ref<Fact> new_fact(FactType* type, const std::vector<Item>& items){
// 	uint32_t length = _resolve_fact_len(items.size(), type);	
// 	Fact* fact = alloc_fact(length);
// 	return _new_fact(fact, type, items.data(), items.size(), length);
// }

std::vector<Member> Fact::get_members() const{
  std::vector<Member> out;
  out.reserve(this->length);
  for(int i=0; i < this->length; i++){
    out.push_back(this->get(i));
  }
  return out;
}

// std::vector<Member*> Fact::get_member_ptrs() const{
//   std::vector<Member*> out;
//   out.reserve(this->length);
//   for(int i=0; i < this->length; i++){
//     Item* item = 
//     out.push_back(this->get(i););
//   }
//   return out;
// }

// inline size_t Fact::size() const{
// 	return this->length;
// }

void _fill_fact_slice_copy(Fact* src, Fact* dest, 
													 size_t start, size_t end,
													 uint8_t copy_kind=COPY_SHALLOW,
													 AllocBuffer* buffer=nullptr){
	// size_t length = end-start;

	// new (dest) Fact(nullptr, length);

	// _init_fact(dest, length, nullptr);	
	size_t src_end = std::min(size_t(src->length), end);

	int j=0;
	int i=start;
	for(; i < src_end; i++, j++){
		Item item = src->get(i);
	
		if(copy_kind >= COPY_DEEP  && 
			 item.get_t_id() == T_ID_FACT && 
			 item.val != 0 && 
			 (!item.is_ref() || copy_kind == COPY_DEEP_REFS) ){
			 // (item_fact = item.as_fact())->immutable){

				Fact* item_fact = item.as_fact();
				ref<Fact> fact_copy = item_fact->copy(copy_kind, buffer);

				// cout << "BREF COUNT" << fact_copy->get_refcount() << endl;
				// cout << "j=" << j << " ITEM FACT:" << item_fact << endl;
				dest->set_unsafe(j, fact_copy);

				// cout << "AREF COUNT" << fact_copy->get_refcount() << endl;

				// cout << "?? " << dest->get(j) << endl;
			// if(item_fact->immutable){
				// TODO
			// }
		}else{
			// Everything else can be copied w/o type checking or recursion;
			dest->set_unsafe(j, item);

			// if(!item.is_primitive() && item.borrows){
			// 	((CRE_Obj*) item.val)->inc_ref();
			// }
		}
		
	}
	if(i < end){
		_zfill_fact(dest, j, end-start);
	}

	dest->immutable = src->immutable;
	dest->ensure_unique_id();

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


// ref<Fact> Fact::slice_into(Fact* new_fact, int _start, int _end, bool deep_copy){
	
// 	// cout << "<< start: " << uint64_t(start) <<
// 	// 				"<< end: " << uint64_t(end) <<
// 	// 	      "<< LENGTH: " << uint64_t(length) << endl;
// 	auto [start, end] = _format_slice(_start, _end);
// 	_fill_fact_slice_copy(this, new_fact, start, end);
// 	// cout << "ALLOC ADDR: " << uint64_t(new_fact) << endl;
	
// 	return new_fact;
// }

// ref<Fact> Fact::slice_into(AllocBuffer& buffer, int _start, int _end, bool deep_copy){
// 	auto [start, end] = _format_slice(_start, _end);
// 	size_t length = end-start;
// 	bool did_malloc = false;
// 	Fact* new_fact = (Fact *) buffer.alloc_bytes(SIZEOF_FACT(length), did_malloc);
// 	_fill_fact_slice_copy(this, new_fact, start, end);
// 	if(!did_malloc){
// 		new_fact->alloc_buffer = &buffer;
// 		buffer.inc_ref();
// 	}
// 	return new_fact;
// }

ref<Fact> Fact::slice(int _start, int _end, uint8_t copy_kind, 
											AllocBuffer* buffer){
	auto [start, end] = _format_slice(_start, _end);
	ref<Fact> new_fact = alloc_fact(nullptr, end-start, buffer);
	// Fact* new_fact = (Fact *) malloc(SIZEOF_FACT(length));
	_fill_fact_slice_copy(this, new_fact, start, end, copy_kind, buffer);
	return new_fact;
}

// ref<Fact> Fact::copy_into(AllocBuffer& buffer, bool deep_copy){
// 	bool did_malloc = false;
// 	Fact* new_fact = (Fact *) buffer.alloc_bytes(SIZEOF_FACT(this->length), did_malloc);
// 	_fill_fact_slice_copy(this, new_fact, 0, this->length);
// 	if(!did_malloc){
// 		new_fact->alloc_buffer = &buffer;
// 		buffer.inc_ref();
// 	}
// 	return new_fact;
// }

ref<Fact> Fact::copy(uint8_t copy_kind, 
										 AllocBuffer* buffer){
	// Fact* fact = () _alloc_fact(_length);
	// AllocBuffer buffer = AllocBuffer(SIZEOF_FACT(this->length), true);
	// cout << "MALLOC" << SIZEOF_FACT(this->length) << endl; 
	// Fact* new_fact = (Fact *) malloc(SIZEOF_FACT(this->length));
	ref<Fact> new_fact = alloc_fact(this->type, this->length, buffer);
	_fill_fact_slice_copy(this, new_fact, 0, this->length, copy_kind, buffer);
	// new_fact->type = this->type;
	// Fact* new_fact = this->slice_into(buffer, 0, this->length, deep_copy);
	// ensure_unique_id();
	return new_fact;
}

std::string Fact::get_unique_id(){
	// std::stringstream ss;
	// FactType* type = fact->type;
	// int unique_id_index = unique_id_index;//get_unique_id_index(type);
	// cout << "unique_id_index: " << type->name << " " << unique_id_index  << endl ;
	if(unique_id_index != -1){
		// cout << "unique_id_index: " << unique_id_index << endl ;
		Item item = get(unique_id_index);

		if(item.get_t_id() == T_ID_STR){
			// UnicodeItem uitem = std::bit_cast<UnicodeItem>(item);
			return std::string(item.data, item.get_length());
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

	bool any_prev = false;
	for(int i=0; i < L; i++){
		bool mbr_has_type = type != nullptr && i < type->members.size();

		const Item& item = get(i);
		// cout << "HERE: " << item.is_wref() << endl;// << uint64_t(item.as_fact()->type) << endl;

		// cout << i << " mbr_has_type: " << mbr_has_type << ", Undef:" << item.is_undef() << endl;

		if(!mbr_has_type || 
			 (mbr_has_type && !item.is_undef()) ||
			 verbosity > DEFAULT_VERBOSITY){

			// Member name part
			if(mbr_has_type){
				MemberSpec* mbr_spec = &type->members[i];
				if(mbr_spec->get_flag(BIFLG_VERBOSITY) >= verbosity){
					continue;
				}
				if(any_prev) ss << ", ";
				if(mbr_spec->name.length() > 0){
					ss << mbr_spec->name << "=";
				}
			}else{
				if(any_prev) ss << ", ";
			}
			
			// Value Part
			ss << item_to_string(item);
			any_prev = true;
			
		}
		
	}
	ss << ")";  
  
	return ss.str();
}


// void fact_dtor(Fact* fact){
// 	cout << "DTOR" << endl;
// 	for(size_t i=0; i < fact->length; i++){
// 		Item item = fact->get(i);
// 		cout << "REF COUNT: " << ((CRE_Obj*) item.val)->get_refcount() << endl;
// 		item.release();



// 		// item.~Item();
// 		// if(item.get_t_id() == T_ID_OBJ){
// 		// 	ObjItem* o_item = (ObjItem*) &item;
// 		// 	((CRE_Obj*) o_item->data)->dec_ref();
// 		// 	// CRE_decref((CRE_Obj*) o_item->data);	
// 		// }
// 	}
// 	if(fact->alloc_buffer != (void *) NULL){
// 		fact->alloc_buffer->dec_ref();
// 		// CRE_decref((CRE_Obj*) fact->alloc_buffer);
// 	}
// 	free(fact);
// }


std::ostream& operator<<(std::ostream& out, Fact* fact){
	return out << fact->to_string();
}
std::ostream& operator<<(std::ostream& out, ref<Fact> fact){
	return out << fact.get()->to_string();
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
//     item.get_t_id() = T_ID_FACT;
//     // cout << "FACT TO ITEM:" << fact << endl;
//     return item;
// }

// Item to_item(Fact* arg) { return fact_to_item(arg);}



// -------------------------------------------------------
// : Fact Hashing 

/**
   * @brief The hash function for Facts and FactViews. Incorperates
   *  the hash of each Member in the fact and its index using XORs 
   *  so that calling `set()` on the fact updates its hash without
   *  rescanning every Member.
   * 
   * @param ind The member's index 
   * @param val The member's value
   * @return void 
   */
uint64_t _hash_fact_range(const Fact* x, int64_t start, int64_t end){

	// Use signed integers so that multiplication overflows wrap 
	//  instead of truncating.
	// int64_t constexpr fnv_prime = 1099511628211ULL;
  // int64_t constexpr fnv_offset_basis = 14695981039346656037ULL;

  // cout << "Fact: " << x << endl;
 	int64_t hash = FNV_BASIS; //^ (end-start * fnv_prime);

 	// Always initialize hash w/ length
 	hash = hash ^ (end-start)*FNV_PRIME;

  for(int64_t i=start; i < end; i++){
      const Member& mbr = x->get(i);

      // Cast int64_t so multiply wraps instead of trucating.
      hash ^= MBR_HASH(mbr.hash, i);//mbr.hash * (i+1) * FNV_PRIME;
  }

  // NOTE: Since hashes changing along w/ set(), forcing non-zero hashes
  //  could cause a rare edge case violation to commutativity. So we
  //  shouldn't do the following, and we shouldn't need to since we
  //  are not using lazy hashing, so zero doesn't indicated inintialized.
  // if(hash == 0){
  // 	hash = FNV_BASIS;
  // }

  return uint64_t(hash); 
}


uint64_t CREHash::operator()(Fact* x) {
	if(x == nullptr) return 0;
	return x->hash;
		// NOTE: Fact's `hash` field is always up to date so the logic below
		//   is unecessary, and could cause issues, since 0 is a legitimate hash.
    // if(x->hash != 0){
    //     return x->hash;
    // }   
    // uint64_t hash = _hash_fact_range(x, 0, x->length);
    // x->hash = hash;
    // return hash;
}

uint64_t CREHash::operator()(const FactView& x) const{
    return _hash_fact_range((const Fact*) x.fact, x.start, x.end_);

    
}

// -------------------------------------------------------
// : Fact Equals 


bool item_eq(const Item& ia, const Item& ib){
	// cout << "ITEM A: " << ia << " ITEM B: " << ib << endl;
	// cout << "HASH A: " << CREHash{}(ia) << " HASH B: " << CREHash{}(ib) << endl;
	if(ia.get_t_id() != ib.get_t_id()) return false;
	switch (ia.get_t_id()) {
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
		const Item& ia = this->get(i);
		const Item& ib = other.get(i);
		if(!item_eq(ia, ib)) return false;
	}
	return true;
};

bool FactView::operator==(const FactView& other) const {
	if(this->size() != other.size()) return false;
	const Fact* fact_a = (const Fact*) this->fact;
	const Fact* fact_b = (const Fact*) other.fact;
	for(size_t i=0; i < this->size(); i++){
		const Item& ia = fact_a->get(this->start + i);
		const Item& ib = fact_b->get(other.start + i);
		if(!item_eq(ia, ib)) return false;
	}
	return true;
}

} // NAMESPACE_END(cre)




