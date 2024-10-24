#ifndef _CRE_VECTORIZER_H_
#define _CRE_VECTORIZER_H_


#include "../include/hash.h"
#include "../include/alloc_buffer.h"
#include "../include/fact.h"


// struct FactOffsetPtr {

// };


struct Vectorizer : public CRE_Obj{
	// -- Members --

	AllocBuffer* buffer; 
	// Nominal Case

	// HashMap<Fact*, int64_t> slot_map = {}

	HashMap<FactView, size_t> slot_map = {};
	std::vector<Fact*> inv_slot_map = {};
	// HashMap<FactSlice<0,-1>, int64_t> slot_map = {};
	// std::vector<HashMap<FactSlice<-2,-1>, int64_t>> nominal_maps;
	std::vector<HashMap<Item, int64_t>> nominal_maps = {};

	// std::vector<FactSlice<0,-1>> inv_slot_map = {};
	std::vector<vector<Item>> inv_nominal_maps = {};


	// One-hot-case
	// HashMap<Fact*, int64_t> fact_map = {};
	// std::vector<Fact*> inv_fact_map = {};

	// std::vector<uint32_t> nominals;
	// std::vector<float> continous;

	bool one_hot_nominals = true;
	bool encode_missing = true;

	// -- Methods --
	Vectorizer(size_t buffer_size=0);

	std::tuple<std::vector<uint64_t>, std::vector<double>>
		apply(FactSet* fs);

	size_t _get_head_slot(Fact* fact);
	size_t _get_nominal_enc(size_t slot, const Item& val_item);
	// void insert_onehot(Fact* fact);
};


#endif /*_CRE_VECTORIZER_H_ */
