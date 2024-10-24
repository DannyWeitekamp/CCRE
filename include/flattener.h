#include <vector>
#include <map>
#include "../include/cre_obj.h"
#include "../include/factset.h"


// Maybe template
struct IncrementalProcessor : public CRE_Obj {
	FactSet* input; //Make ref
	size_t change_queue_head;

	IncrementalProcessor(FactSet* _input);
};

const uint8_t TRIPLE_ORDER_SPO = 1;
const uint8_t TRIPLE_ORDER_PSO = 2;

std::vector<FlagGroup> _format_flags(
	const vector<HashMap<std::string, Item>>& target_flag_lst
);
std::vector<FlagGroup> _format_flags(
	const vector<HashMap<std::string, Item>>& target_flag_lst
);

std::vector<FlagGroup> default_flags();


struct Flattener : public IncrementalProcessor {
// -- Members --
	FactSet* output;

	// Which flags to target as criteria for flattening
	std::vector<FlagGroup> flag_groups;	
	std::map<FactType*, vector<uint16_t>> type_to_member_inds = {};
	FactSetBuilder builder;

	const uint8_t triple_order;
	const uint8_t subj_ind;
	const uint8_t pred_ind;
	const bool subj_as_atom;
	

// -- Methods --
	Flattener(FactSet* _input=nullptr,
 		  const std::vector<FlagGroup>& _flag_groups=default_flags(),
	 	  uint8_t _triple_order = TRIPLE_ORDER_SPO,
	 	  uint8_t _subj_as_atom = false
	);

	// template<typename T>
	Flattener(FactSet* _input,
			  const HashMap<std::string, Item>& target_flags,
	 	  uint8_t _triple_order = TRIPLE_ORDER_SPO,
	 	  uint8_t _subj_as_atom = false
	);
	Flattener(FactSet* _input,
			  const vector<HashMap<std::string, Item>>& target_flags,
	 	  uint8_t _triple_order = TRIPLE_ORDER_SPO,
	 	  uint8_t _subj_as_atom = false
	);
	// Flattener(FactSet* _input,
 	// 	  const HashMap<std::string, Item>& target_flags={
	//  		  	{{"visible",  to_item(true)}}
	//  	  },
	//  	  uint8_t _triple_order = TRIPLE_ORDER_SPO,
	//  	  uint8_t _subj_as_atom = false
	// );

	// Flattener(FactSet* _input, 
	// 	const vector<HashMap<std::string, Item>>& target_flag_lst={{
	//  		  	{{"visible",  to_item(true)}}
	//  	 }},
	// 	uint8_t _triple_order = TRIPLE_ORDER_SPO,
	// 	uint8_t _subj_as_atom = false
	// 	);

	std::vector<uint16_t>* get_member_inds(FactType* type);
	size_t _calc_buffer_size();
	size_t _flatten_fact(Fact* in_fact);

	// size_t _calc_in_out_info(
	// 	vector<std::tuple<Fact*, std::vector<uint16_t>*, uint32_t>>& infos,
	// 	bool use_atom
	// );
	
	size_t _update_init();
};
