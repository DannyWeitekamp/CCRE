
#include <math.h>
#include "../include/vectorizer.h"
#include "../include/item.h"
#include "../include/fact.h"
#include "../include/factset.h"

Vectorizer::Vectorizer(uint64_t max_heads){
	buffer = new AllocBuffer(max_heads*SIZEOF_FACT(3));
	slot_map.reserve(max_heads);
	inv_slot_map.reserve(max_heads);
}

size_t Vectorizer::_get_head_slot(Fact* fact){
	size_t head_slot = slot_map.size();
	
	FactView fv = FactView(fact, 0, -1);

	
	auto [it, inserted] = slot_map.try_emplace(fv, head_slot);
	if(inserted){
		// Hot swap a sliced copy the fact into the emplaced key   
		FactView* fv_ptr = &(it->first);
		Fact* fact_slice = fact->slice_into(*buffer, 0, fact->length-1, true);
		fv_ptr->fact = fact_slice;
		inv_slot_map.push_back(fact_slice);
	}else{
		head_slot = (*it).second;
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
    return head_slot;
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

	std::vector<uint64_t> nom_vec(slot_map.size() + fs->size, 0);
	std::vector<double> flt_vec(slot_map.size() + fs->size, NAN);

	for (auto it = fs->begin(); it != fs->end(); ++it) {
		Fact* fact = (*it);
		size_t head_slot = this->_get_head_slot(fact);
		Item val_item = *fact->get(fact->length-1);
		size_t nom_enc = this->_encode_item(val_item);
		nom_vec[head_slot] = nom_enc;

		// cout << "Fact=" << fact << 
		// 	    ", slot=" << head_slot << ", nom=" << nom_enc << endl;
	}

	// Cut off any unused bits
	nom_vec.resize(slot_map.size());
	flt_vec.resize(slot_map.size());
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
