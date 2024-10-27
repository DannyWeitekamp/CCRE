
#include <math.h>
#include "../include/hash.h"
#include "../include/vectorizer.h"
#include "../include/item.h"
#include "../include/fact.h"
#include "../include/factset.h"


uint64_t CREHash::operator()(const UintPair& x) const {
	return *((uint64_t*)&x);
}

Vectorizer::Vectorizer(uint64_t max_heads, bool _one_hot_nominals, bool _encode_missing) :
	IncrementalProcessor(0), 
	one_hot_nominals(_one_hot_nominals),
	encode_missing(_encode_missing)
{
	buffer = new AllocBuffer(max_heads*SIZEOF_FACT(3));
	nom_slot_map.reserve(max_heads);
	flt_slot_map.reserve(max_heads);
	inv_nom_slot_map.reserve(max_heads);
	inv_flt_slot_map.reserve(max_heads);

}

size_t Vectorizer::_get_nom_slot(Fact* fact){
	// Typically slots are allocated by the head (all but the last item) 
	// 	of each grounded predicate (usually a triple in SPO or PSO format)
	size_t slot = nom_slot_map.size();
	FactView fv = FactView(fact, 0, -1);
	if(fact->length == 1){
		// If we have a fact with length 1
		//  then don't slice off anything; 
		fv = FactView(fact, 0, 1);
	}

	auto [it, inserted] = nom_slot_map.try_emplace(fv, slot);
	if(inserted){
		// Hot-swap pointer to sliced copy of fact into the emplaced key. Copy
		//  avoids borrowing a reference to 'fact' and possibly leaking its
		//  AllocBuffer (in the case where it wasn't alloced with malloc);
		FactView* fv_ptr = &(it->first);
		Fact* fact_slice = fact->slice_into(*buffer, 0, fv.end_, true);
		fv_ptr->fact = fact_slice;

		// The sliced copy is used directly by inverse maps
		inv_nom_slot_map.push_back(fact_slice);
	}else{
		slot = it->second;
	}

	return slot;
}	

size_t Vectorizer::_get_flt_slot(Fact* fact){	
	/* Same as _get_nom_slot but w/o L=1 case */
	size_t slot = flt_slot_map.size();
	FactView fv = FactView(fact, 0, -1);

	auto [it, inserted] = flt_slot_map.try_emplace(fv, slot);
	if(inserted){
		FactView* fv_ptr = &(it->first);
		Fact* fact_slice = fact->slice_into(*buffer, 0, fv.end_, true);
		fv_ptr->fact = fact_slice;
		inv_flt_slot_map.push_back(fact_slice);
	}else{
		slot = it->second;
	}
	return slot;
}

size_t Vectorizer::_encode_item(const Item& val_item){
	size_t enc = enumerize_map.size();
	auto [it, inserted] = enumerize_map.try_emplace(val_item, enc);
	if(inserted){
		inv_enumerize_map.push_back(val_item);
	}else{
		enc = it->second;
	}
	return enc;
}


size_t Vectorizer::_get_one_hot_slot(size_t slot, size_t enc){
	UintPair pair = {uint32_t(slot), uint32_t(enc)};
	size_t oh_slot = one_hot_map.size();
	auto [it, inserted] = one_hot_map.try_emplace(pair, oh_slot);
	if(inserted){
		inv_one_hot_map.push_back(pair);
	}else{
		oh_slot = it->second;
	}

	return oh_slot;
}

void Vectorizer::_init_new(FactSet* fs){
	input = fs;
	size_t nom_size = one_hot_nominals ? one_hot_map.size() : nom_slot_map.size();
	nom_vec = std::vector<uint64_t>(nom_size + fs->size(), 0);
	flt_vec = std::vector<double>(flt_slot_map.size() + fs->size(), NAN);

	if(encode_missing and one_hot_nominals){
		// Set=1 any slots associated with missing values  
		for (const auto& [key, slot] : nom_slot_map) {
			nom_vec[_get_one_hot_slot(slot, 0)] = 1;
		}
	}
}

void Vectorizer::_map_fact(Fact* fact){
	// Default =1 for existence of L=1 fact 
	size_t nom_enc = 1; 
	bool val_is_nominal = true;
	Item* last_item = nullptr;
	if(fact->length > 1){
		last_item = fact->get(fact->length-1);
		if(last_item->t_id == T_ID_FLOAT){
			val_is_nominal = false;
		}else{
			nom_enc = this->_encode_item(*last_item);	
		}
	}

	if(val_is_nominal){
		size_t slot = this->_get_nom_slot(fact);
		if(one_hot_nominals){
			size_t oh_slot = this->_get_one_hot_slot(slot, nom_enc);
			nom_vec[oh_slot] = 1;

			
			if(encode_missing){
				// Set=0 the one-hot for 'slot' is missing
				//  overwriting previous Set=1
				nom_vec[_get_one_hot_slot(slot, 0)] = 0;
			}
		}else{
			nom_vec[slot] = nom_enc;	
		}
	}else{
		cout << "Map float" << last_item->as_float() << endl;
		size_t slot = this->_get_flt_slot(fact);
		flt_vec[slot] = last_item->as_float();
	}
}


std::tuple<std::vector<uint64_t>, std::vector<double>> 
	Vectorizer::apply(FactSet* fs)
{		
	_init_new(fs);
	for (auto it = fs->begin(); it != fs->end(); ++it) {
		Fact* fact = (*it);
		_map_fact(fact);
	}

	// Cut off any unused bits
	size_t nom_size = one_hot_nominals ? one_hot_map.size() : nom_slot_map.size();
	nom_vec.resize(nom_size);
	flt_vec.resize(flt_slot_map.size());
	return std::make_tuple(nom_vec, flt_vec);
}
