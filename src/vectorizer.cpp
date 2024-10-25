
#include <math.h>
#include "../include/hash.h"
#include "../include/vectorizer.h"
#include "../include/item.h"
#include "../include/fact.h"
#include "../include/factset.h"


uint64_t CREHash::operator()(const SlotPair& x) const {
	return *((uint64_t*)&x);
}

Vectorizer::Vectorizer(uint64_t max_heads){
	buffer = new AllocBuffer(max_heads*SIZEOF_FACT(3));
	slot_map.reserve(max_heads);
	inv_nom_slot_map.reserve(max_heads);
	inv_flt_slot_map.reserve(max_heads);
}

SlotPair Vectorizer::_get_head_slots(Fact* fact, bool val_is_nominal){	
	// Typically slots are allocated by the head (all but the last item) 
	// 	of each grounded predicate (usually a triple in SPO or PSO format)
	FactView fv = FactView(fact, 0, -1);
	if(fact->length == 1){
		// If we have a fact with length 1 (probably an atom)
		//  then don't slice off anything; 
		fv = FactView(fact, 0, 1);
	}


	SlotPair head_slots = val_is_nominal ? SlotPair(nom_size, -1) : SlotPair(-1, flt_size);
	auto [it, inserted] = slot_map.try_emplace(fv, head_slots);
	if(inserted){
		// Hot-swap pointer to sliced copy of fact into the emplaced key. Copy
		//  avoids borrowing a reference to 'fact' and possibly leaking its
		//  AllocBuffer (in the case where it wasn't alloced with malloc);
		FactView* fv_ptr = &(it->first);
		Fact* fact_slice = fact->slice_into(*buffer, 0, fv.end_, true);
		fv_ptr->fact = fact_slice;

		// The sliced copy is used directly by inverse maps
		if(val_is_nominal){
			inv_nom_slot_map.push_back(fact_slice);
		}else{
			inv_flt_slot_map.push_back(fact_slice);
		}
		
	}else{
		// Hot-swap slot values if necessary
		SlotPair* prev_head_slots = &(it->second);
		if(val_is_nominal){
			if(prev_head_slots->nom_slot == uint32_t(-1)){
				prev_head_slots->nom_slot = nom_size;
			}
		}else{
			if(prev_head_slots->flt_slot == uint32_t(-1)){
				prev_head_slots->flt_slot = flt_size;
			}
		}
		head_slots = *prev_head_slots;
		head_slots = it->second;
	}

	if(val_is_nominal){
		nom_size++;
	}else{
		flt_size++;
	}
	

	/* Find-Insert approach -- slower */
	// auto sm_itr = slot_map.find(fv);
	// if (sm_itr != slot_map.end()) {
	// 	head_slot = (*sm_itr).second;
    // }else{
    // 	// Make a sliced copy of all but the last element of fact
    // 	Fact* fact_slice = fact->slice_into(*buffer, 0, fact->length-1, true);
    // 	fv = FactView(fact_slice, 0, fact_slice->length);
    // 	head_slot = slot_map.size();
	//     slot_map[fv] = head_slot;
	//     inv_slot_map.push_back(fact_slice);
    // }
    /* End Try Find-Insert */
    return head_slots;
}

size_t Vectorizer::_encode_item(const Item& val_item){
	size_t enc = enumerize_map.size();

	auto [it, inserted] = enumerize_map.try_emplace(val_item, enc);

	if(inserted){
		inv_enumerize_map.push_back(val_item);
	}else{
		enc = (*it).second;
	}

	/* Find-Insert approach -- maybe slower */
    // auto it = enumerize_map.find(val_item);
    // if (it != enumerize_map.end()) {
	// 	enc = (*it).second;
	// }else{
	// 	enc = enumerize_map.size();
	// 	enumerize_map[val_item] = enc;
	// 	inv_enumerize_map.push_back(val_item);
	// }
	/* End Try Find-Insert */
	return enc;
}

std::tuple<std::vector<uint64_t>, std::vector<double>> 
	Vectorizer::apply(FactSet* fs){

	std::vector<uint64_t> nom_vec(nom_size + fs->size, 0);
	std::vector<double> flt_vec(flt_size + fs->size, NAN);

	for (auto it = fs->begin(); it != fs->end(); ++it) {
		Fact* fact = (*it);

		bool val_is_nominal = true;
		size_t nom_enc = 1; // Default for existence of L=1 fact
		Item* last_item = nullptr;
		if(fact->length > 1){
			Item* last_item = fact->get(fact->length-1);
			if(last_item->t_id == T_ID_FLOAT){
				val_is_nominal = false;
			}else{
				// cout << "LAST ITEM: " << *last_item << endl;
				nom_enc = this->_encode_item(*last_item);	
			}
		}

		SlotPair head_slots = this->_get_head_slots(fact, val_is_nominal);

		// Item val_item = *fact->get(fact->length-1);
		// size_t nom_enc = this->_encode_item(val_item);
		// cout << "HEAD_SLOTS(" << head_slots.nom_slot << ", " << head_slots.flt_slot << ")" << endl;
		if(val_is_nominal){
			// cout << "NOM ENC: " << nom_enc << endl;
			nom_vec[head_slots.nom_slot] = nom_enc;
		}else{
			flt_vec[head_slots.flt_slot] = item_get_float(last_item);
		}
		

		// cout << "Fact=" << fact << 
		// 	    ", slot=" << head_slot << ", nom=" << nom_enc << endl;
	}

	// Cut off any unused bits
	nom_vec.resize(nom_size);
	flt_vec.resize(flt_size);
	return std::make_tuple(nom_vec, flt_vec);
}


// void Vectorizer::insert_onehot(Fact* fact){
// 	int slot;
// 	auto itr = fact_map.find(fact);
// 	if (itr != fact_map.end()) {
// 		slot = *itr;
//     }else{
//     	slot = fact_map.size();
// 	    fact_map[fact] = slot;
// 	    inv_fact_map.push_back(fact);
//     }

//     // nominals
// }
