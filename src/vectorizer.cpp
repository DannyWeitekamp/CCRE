
#include <math.h>
#include "../include/vectorizer.h"
#include "../include/item.h"
#include "../include/fact.h"
#include "../include/factset.h"

Vectorizer::Vectorizer(uint64_t max_heads){
	buffer = new AllocBuffer(max_heads*SIZEOF_FACT(3));
	slot_map.reserve(max_heads);
	inv_slot_map.reserve(max_heads);
	nominal_maps.reserve(max_heads);
	inv_nominal_maps.reserve(max_heads);
}

size_t Vectorizer::_get_head_slot(Fact* fact){
	size_t head_slot;
	FactView fv = FactView(fact, 0, -1);

	auto sm_itr = slot_map.find(fv);
	if (sm_itr != slot_map.end()) {
		head_slot = (*sm_itr).second;
    }else{
    	// Make a sliced copy of all but the last element of fact
    	Fact* fact_slice = fact->slice_into(*buffer, 0, fact->length-1, true);
    	// Fact* fact_slice = (Fact*) buffer->alloc_bytes(SIZEOF_FACT(fact->length-1));
    	// cout << "fact_slice:" << fact_slice << endl; 

    	fv = FactView(fact_slice, 0, fact_slice->length);

    	head_slot = slot_map.size();
	    slot_map[fv] = head_slot;
	    inv_slot_map.push_back(fact_slice);
	    nominal_maps.push_back({});
	    inv_nominal_maps.push_back({});
    }
    return head_slot;
}

size_t Vectorizer::_get_nominal_enc(size_t slot, const Item& val_item){
	size_t nom;
    auto& nominal_map = nominal_maps[slot];
    auto& inv_nominal_map = inv_nominal_maps[slot];
    
    // cout << "VALUE ITEM: " << val_item;
    auto nm_itr = nominal_map.find(val_item);
    if (nm_itr != nominal_map.end()) {
		nom = (*nm_itr).second;
	}else{
		nom = nominal_map.size();
		nominal_map[val_item] = nom;
		inv_nominal_map.push_back(val_item);
	}
	return nom;
}

std::tuple<std::vector<uint64_t>, std::vector<double>> 
	Vectorizer::apply(FactSet* fs){

	std::vector<uint64_t> nom_vec(slot_map.size() + fs->size, 0);
	std::vector<double> flt_vec(slot_map.size() + fs->size, NAN);

	for (auto it = fs->begin(); it != fs->end(); ++it) {
		Fact* fact = (*it);
		size_t head_slot = this->_get_head_slot(fact);
		Item val_item = *fact->get(fact->length-1);
		size_t nom_enc = this->_get_nominal_enc(head_slot, val_item);

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
