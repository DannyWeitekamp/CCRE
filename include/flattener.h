#ifndef _CRE_FLATTENER_H_
#define _CRE_FLATTENER_H_

#include <vector>
#include <map>
#include "../include/cre_obj.h"
#include "../include/ref.h"
#include "../include/factset.h"
#include "../include/incr_processor.h"


// Subject Verb Object
const uint8_t TRIPLE_ORDER_SVO = 1;

// Verb Subject Object
const uint8_t TRIPLE_ORDER_VSO = 2;

std::vector<FlagGroup> _format_flags(
	const std::vector<HashMap<std::string, Item>>& target_flag_lst
);
std::vector<FlagGroup> _format_flags(
	const std::vector<HashMap<std::string, Item>>& target_flag_lst
);

// std::vector<FlagGroup> default_flags();



struct Flattener : public IncrementalProcessor {
	static std::vector<FlagGroup> default_flags;
		// {FlagGroup()};


// -- Members --
	ref<FactSet> output;

	// Which flags to target as criteria for flattening
	std::vector<FlagGroup> flag_groups;	
	std::map<FactType*, std::vector<uint16_t>> type_to_member_inds = {};
	FactSetBuilder builder;

	
	const bool use_vars;
	const bool add_exist_stubs;
	const uint8_t triple_order;
	const uint8_t subj_ind;
	const uint8_t verb_ind;
	
	

// -- Methods --
	Flattener(FactSet* _input,
 		  const std::vector<FlagGroup>& _flag_groups,
	 	  bool _use_vars = false,
	 	  bool _add_exist_stubs = false,
	 	  uint8_t _triple_order = TRIPLE_ORDER_SVO
	);

	Flattener(FactSet* _input=nullptr,
	 	  bool _use_vars = false,
	 	  bool _add_exist_stubs = false,
	 	  uint8_t _triple_order = TRIPLE_ORDER_SVO
	);
	// template<typename T>
	Flattener(FactSet* _input,
			  const HashMap<std::string, Item>& target_flags,
	 	  bool _use_vars = false,
	 	  bool _add_exist_stubs = false,
	 	  uint8_t _triple_order = TRIPLE_ORDER_SVO
	);
	Flattener(FactSet* _input,
			  const std::vector<HashMap<std::string, Item>>& target_flags,
	 	  bool _use_vars = false,
	 	  bool _add_exist_stubs = false,
	 	  uint8_t _triple_order = TRIPLE_ORDER_SVO
	);
	// Flattener(FactSet* _input,
 	// 	  const HashMap<std::string, Item>& target_flags={
	//  		  	{{"visible",  to_item(true)}}
	//  	  },
	//  	  uint8_t _triple_order = TRIPLE_ORDER_SVO,
	//  	  uint8_t _add_exist_stubs = false
	// );

	// Flattener(FactSet* _input, 
	// 	const vector<HashMap<std::string, Item>>& target_flag_lst={{
	//  		  	{{"visible",  to_item(true)}}
	//  	 }},
	// 	uint8_t _triple_order = TRIPLE_ORDER_SVO,
	// 	uint8_t _add_exist_stubs = false
	// 	);

	std::vector<uint16_t>* get_member_inds(FactType* type);
	size_t _calc_buffer_size();
	std::vector<ref<Var>> _make_fact_vars(FactSet* input);


	size_t _fact_to_var_pairs(
		Fact* __restrict in_fact,
		const std::vector<ref<Var>>& var_vec
	);
	size_t _fact_to_wme_triples(Fact* in_fact);
	size_t _update_init();
	ref<FactSet> apply(FactSet* fs);
};

#endif /* #ifndef _CRE_FLATTENER_H_ */
