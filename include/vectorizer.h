#ifndef _CRE_VECTORIZER_H_
#define _CRE_VECTORIZER_H_


#include "../include/hash.h"
#include "../include/alloc_buffer.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/incr_processor.h"


// struct FactOffsetPtr {

// };

struct UintPair{
	// struct {
	uint32_t first; 
	uint32_t second;
	// }
	// uint64_t data;

	UintPair(uint32_t a, uint32_t b) : first(a), second(b) {};
	bool operator==(const UintPair& other) const {
	 	return (first == other.first) && (second == other.second);
	}
};


struct Vectorizer : public IncrementalProcessor{
	// -- Members --

	// A buffer for copying fact slices into
	//   size is determined by 'max_heads'
	AllocBuffer* buffer; 

	// Map: Fact Head -> nominal slot
	HashMap<FactView, size_t> nom_slot_map = {};
	std::vector<Fact*> inv_nom_slot_map = {};
	
	// Map: Fact Head -> float slot
	std::vector<Fact*> inv_flt_slot_map = {};
	HashMap<FactView, size_t> flt_slot_map = {};

	// Map: Fact Value (last item) -> nominal_encoding
	HashMap<Item, size_t> enumerize_map = {{Item(), 0}};
	std::vector<Item> inv_enumerize_map = {Item()};

	// Map: (slot, nominal_encoding) -> one_hot_slot
	HashMap<UintPair, size_t> one_hot_map = {};
	std::vector<UintPair> inv_one_hot_map = {};

	// The output vectors
	std::vector<uint64_t> nom_vec = {};
	std::vector<double> flt_vec = {};


	bool one_hot_nominals = true;
	bool encode_missing = true;

	// -- Methods --
	Vectorizer(uint64_t max_heads=0,
			   bool _one_hot_nominals=true,
			   bool _encode_missing=false
	);

	std::tuple<std::vector<uint64_t>, std::vector<double>>
		apply(FactSet* fs);

	size_t _get_nom_slot(Fact* fact);
	size_t _get_flt_slot(Fact* fact);
	size_t _encode_item(const Item& val_item);
	size_t _get_one_hot_slot(size_t slot, size_t enc);
	void _init_new(FactSet* fs);
	void _map_fact(Fact* fact);

	Fact* invert(size_t slot, size_t value);
	Fact* invert(size_t slot, double value);
	// void insert_onehot(Fact* fact);
};


#endif /*_CRE_VECTORIZER_H_ */
