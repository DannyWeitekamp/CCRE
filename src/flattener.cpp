#include <bitset>
#include <map>
#include "../include/hash.h"
#include "../include/flattener.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include <execution>
#include <thread>


IncrementalProcessor::IncrementalProcessor(FactSet* _input) :
 	input(_input), change_queue_head(0){
}



// ------------------------------------------------------------
// Constructor

std::vector<FlagGroup> _format_flags(
	const HashMap<std::string, Item>& target_flags)
{
	std::vector<FlagGroup> flag_groups = {FlagGroup(target_flags)};
	return flag_groups;
}

std::vector<FlagGroup> _format_flags(
	const std::vector<HashMap<std::string, Item>>& target_flag_lst)
{
	std::vector<FlagGroup> flag_groups = {};
	flag_groups.reserve(target_flag_lst.size());
	for(auto target_flags : target_flag_lst){
		flag_groups.push_back(FlagGroup(target_flags));
	}
	return flag_groups;
}

std::vector<FlagGroup> default_flags(){
	HashMap<std::string, Item> def_hash_map= {
		{{"visible",  to_item(true)}}
	};
	std::vector<FlagGroup> flag_groups = {FlagGroup(def_hash_map)};
	return flag_groups;
}

// Main Constructor
Flattener::Flattener(
		FactSet* _input, 
		const vector<FlagGroup>& _flag_groups,
		uint8_t _triple_order,
		uint8_t _subj_as_atom) :
	IncrementalProcessor(_input),
	flag_groups(_flag_groups),
	triple_order(_triple_order), subj_as_atom(_subj_as_atom),
	subj_ind(_triple_order == TRIPLE_ORDER_SPO ? 0 : 1),
	pred_ind(_triple_order == TRIPLE_ORDER_SPO ? 1 : 0)
	{
}

Flattener::Flattener(FactSet* _input,
			  const HashMap<std::string, Item>& target_flags,
	 	  uint8_t _triple_order,
	 	  uint8_t _subj_as_atom
) : Flattener(_input, _format_flags(target_flags), _triple_order, _subj_as_atom)
{}

Flattener::Flattener(FactSet* _input,
		  const vector<HashMap<std::string, Item>>& target_flags,
 	  uint8_t _triple_order,
 	  uint8_t _subj_as_atom
) : Flattener(_input, _format_flags(target_flags), _triple_order, _subj_as_atom)
{}






// Flattener::Flattener(
// 		FactSet* _input, 
// 		const HashMap<std::string, Item>& target_flags,
// 		uint8_t _triple_order,
// 		uint8_t _subj_as_atom) :

// 		// Delegate constructor
// 		Flattener(_input, 
// 				 _format_flags(target_flags),
// 				 _triple_order, _subj_as_atom
// 				 )
// 	 {
// }

// Flattener::Flattener(
// 		FactSet* _input, 
// 		const std::vector<HashMap<std::string, Item>>& target_flag_lst,
// 		uint8_t _triple_order,
// 		uint8_t _subj_as_atom) :

// 		// Delegate constructor
// 		Flattener(_input, 
// 				 _format_flags(target_flag_lst),
// 				 _triple_order, _subj_as_atom
// 				 )
// 	 {
// }


// ---------------------------------------------------------


std::vector<uint16_t>* Flattener::get_member_inds(FactType* type){
	if(type == nullptr){
		return nullptr;
	}
	auto it = this->type_to_member_inds.find(type);

	if(it == this->type_to_member_inds.end()){
		size_t L = type->members.size();
		
		std::vector<uint16_t> mbr_mask = {};
		mbr_mask.reserve(L);
		for(uint16_t i=0; i < L; i++){
			MemberSpec* mbr_spec = &type->members[i];
			for(auto& flag_group: flag_groups){
				if(mbr_spec->flags.has_flags(flag_group)){
					mbr_mask.push_back(i);
					break;
				}
			}
		}
		auto it2 = type_to_member_inds.insert({type, mbr_mask});
		return &it2.first->second;
	}else{
		return &it->second;
	}
}

size_t Flattener::_calc_buffer_size(){
	size_t _size = 0;
	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;
		size_t L = fact->length;
		auto mbr_inds = this->get_member_inds(fact->type);
		if(mbr_inds != nullptr){
			L = mbr_inds->size();
		}
		_size += this->subj_as_atom*(sizeof(Fact) + sizeof(Item));
		_size += L*(sizeof(Fact) + 3*sizeof(Item));
	}
	return _size;
}


size_t Flattener::_flatten_fact(Fact* __restrict in_fact){
	FactType* __restrict type = in_fact->type;
	auto mbr_inds = this->get_member_inds(type);
		
	// Make Atom

	auto u_ind = get_unique_id_index(type);
	Item id_item = (u_ind != -1 ? 
						*in_fact->get(u_ind) :
						Item(in_fact->f_id)
					);
	

	if(this->subj_as_atom){
		Fact* __restrict atom = builder.next_empty(1);
		atom->length = 1;
		atom->set_unsafe(0, id_item);	
		atom->immutable = true;
		_declare_to_empty(builder.fact_set, atom, 1, NULL);	
		id_item = Item(atom);
		// cout << "ATOM: " << atom << endl;
	}
	
	auto make_preds = [&](size_t ind){
		Fact* __restrict out_fact = builder.next_empty(3);
		out_fact->length = 3;

		out_fact->set_unsafe(subj_ind /* 0 or 1 */, id_item);

		if(type != nullptr && ind < type->members.size()){
			out_fact->set_unsafe(pred_ind /* 1 or 0 */,
			 			type->member_names_as_items[ind]);
		}else{
			out_fact->set(pred_ind /* 1 or 0 */, int(ind));
		}
		out_fact->set_unsafe(2, *in_fact->get(ind));	
		out_fact->immutable = true;
		_declare_to_empty(builder.fact_set, out_fact, 3, NULL);

		// cout << uint64_t(out_fact) <<  " OUT FACT: " << out_fact << " L=" << out_fact->length << endl;
	};
	

	if(mbr_inds != nullptr){
		for(auto ind : *mbr_inds){
			make_preds(ind);
		}	
	}else{
		for(size_t ind=0; ind < in_fact->length; ind++){
			make_preds(ind);
		}
	}	
	return 0;
}


size_t Flattener::_update_init(){
	size_t buffer_size = this->_calc_buffer_size();
	builder = FactSetBuilder(input->size, buffer_size);

	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;
		this->_flatten_fact(fact);
	}
	return 0;
}



// void Flattener::update(){
// 	for (auto it = fs->begin(); it != fs->end(); ++it) {
// 		Fact* fact = *it;

// 	}

// 	if(input != nullptr){

// 	}
// }





/* GARBAGE *.

// PREFETCH IDEA... no significant improvement
size_t Flattener::_update_init(){
	size_t buffer_size = this->_calc_buffer_size();
	this->builder = new FactSetBuilder(input->size, buffer_size);

	
	auto it = input->begin();
	Fact* prev_fact = *it;
	++it;
	for (; it != input->end(); ++it) {
		Fact* fact = *it;
		__builtin_prefetch (fact, 0, 1);
		__builtin_prefetch (((uint8_t*) fact)+128, 0, 1);
		__builtin_prefetch (builder->alloc_buffer->head, 1, 1);
		__builtin_prefetch (builder->alloc_buffer->head+128, 1, 1);
		this->_flatten_fact(prev_fact);
		prev_fact = fact;
	}
	this->_flatten_fact(prev_fact);
	return 0;
}
*/

