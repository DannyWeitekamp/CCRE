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


struct Flattener : public IncrementalProcessor {
// -- Members --
	FactSet* output;

	// Which flags to target as criteria for flattening
	std::vector<FlagGroup> flag_groups;	
	std::map<FactType*, vector<uint16_t>> type_to_member_inds = {};
	FactSetBuilder builder;
	bool use_atom;

// -- Methods --
	Flattener(FactSet* input=nullptr,
 		  const HashMap<std::string, Item>& target_flags={
	 		  	{{"visible",  to_item(true)}}
 		  }
	);

	Flattener(FactSet* _input, 
		const vector<HashMap<std::string, Item>>& target_flag_lst
		);

	std::vector<uint16_t>* get_member_inds(FactType* type);
	size_t _calc_buffer_size();
	size_t _flatten_fact(Fact* in_fact);

	// size_t _calc_in_out_info(
	// 	vector<std::tuple<Fact*, std::vector<uint16_t>*, uint32_t>>& infos,
	// 	bool use_atom
	// );
	
	size_t _update_init();
};
