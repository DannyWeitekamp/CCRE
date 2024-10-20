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


Flattener::Flattener(
	FactSet* _input, 
	const HashMap<std::string, Item>& target_flags) :
	IncrementalProcessor(_input){

	flag_groups = {FlagGroup(target_flags)};
}

Flattener::Flattener(
	FactSet* _input, 
	const vector<HashMap<std::string, Item>>& target_flag_lst) :
	IncrementalProcessor(_input){

	flag_groups = {};
	flag_groups.reserve(target_flag_lst.size());
	for(auto target_flags : target_flag_lst){
		flag_groups.push_back(FlagGroup(target_flags));
	}
}


std::vector<uint16_t>* Flattener::get_member_inds(FactType* type){
	// FactType* type = fact->type;
	auto it = this->type_to_member_inds.find(type);

	if(it == this->type_to_member_inds.end()){
		size_t L = type->members.size();//fact->length;
		
		std::vector<uint16_t> mbr_mask = {};
		mbr_mask.reserve(L);
		for(uint16_t i=0; i < L; i++){
			bool any_okay = false;
			// if(type != nullptr && i < type->members.size()){
			MemberSpec* mbr_spec = &type->members[i];
			// cout << "C" << endl;
			for(auto& flag_group: flag_groups){
				if(mbr_spec->flags.has_flags(flag_group)){
					mbr_mask.push_back(i);
					// any_okay = true;
					break;
				}
			}
			// std::cout << "MBR " << i << ": " << any_okay << std::endl;
		}
		auto it2 = type_to_member_inds.insert({type, mbr_mask});
		return &it2.first->second;
	}else{
		return &it->second;
	}
}

size_t Flattener::_calc_output_size(){
	size_t _size = 0;
	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;
		_size += this->get_member_inds(fact->type)->size();
	}
	return _size;
}



size_t Flattener::_calc_in_out_info(
	vector<std::tuple<Fact*, std::vector<uint16_t>*, uint32_t>>& infos,
	bool use_atom){
	infos.reserve(input->size);

	size_t i = 0;
	size_t offset = 0;
	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;

		size_t len = fact->length;
		std::vector<uint16_t>* member_inds;
		if(fact->type != nullptr){
			member_inds = this->get_member_inds(fact->type);	
			len = member_inds->size();
		}else{
			member_inds = nullptr;
		}
		
		infos.push_back(std::make_tuple(fact, member_inds, offset));

		offset += use_atom*(sizeof(Fact) + 1*sizeof(Item));
		offset += len*(sizeof(Fact) + 3*sizeof(Item));
		 // + this->get_member_inds(fact->type)->size();
		i++;
	}
	return offset;
}


size_t Flattener::_flatten_fact(Fact* in_fact){
	// Fact* in_fact = std::get<0>(info);
	// auto mbr_inds = std::get<1>(info);
	// size_t offset = std::get<2>(info);
	FactType* type = in_fact->type;
	auto mbr_inds = this->get_member_inds(type);
	// uint8_t* pred_data = (data + offset);
		
	// Make Atom
	Fact* atom = builder->next_empty(1);
	atom->length = 1;
	auto u_ind = get_unique_id_index(type);
	if(u_ind != -1){
		atom->set(0, *in_fact->get(u_ind));	
	}else{
		atom->set(0, in_fact->f_id);
	}
	
	_declare_from_empty(builder, atom, 1, NULL);
	atom->immutable = true;

	// size_t f_offset = sizeof(Fact)+1*sizeof(Item);

	auto make_preds = [&](size_t ind){
		Fact* out_fact = builder->next_empty(3);
		// Fact* out_fact = (Fact*) (pred_data + f_offset);//builder.next_empty(3);
		out_fact->length = 3;
		// out += uint64_t(out_fact);
		// f_offset += sizeof(Fact)+3*sizeof(Item);

		out_fact->set(0, atom);

		if(type != nullptr && ind < type->members.size()){
			out_fact->set(1, type->member_names_as_items[ind]);
		}else{
			out_fact->set(1, int(ind));
		}
		out_fact->set(2, *in_fact->get(ind));	
		_declare_from_empty(builder, out_fact, 3, NULL);
		out_fact->immutable = true;
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
}


size_t Flattener::_update_init(){
	// vector<std::tuple<Fact*, std::vector<uint16_t>*, uint32_t>> infos= {};
	size_t size = input->size;
	// size_t buffer_size = this->_calc_in_out_info(infos, true);
	size_t buffer_size = this->_calc_output_size();
	this->builder = new FactSetBuilder(size, buffer_size);

	size_t i = 0;
	uint8_t* data = builder.alloc_buffer->head;

	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;
		this->_flatten_fact(fact);
	}
	// for (size_t i = 0; i < infos.size(); i++) {
	// 	auto info = infos[i];
		
	// }
	// cout << builder.fact_set << endl;
	return 0;
}

// void Flattener::update(){
// 	for (auto it = fs->begin(); it != fs->end(); ++it) {
// 		Fact* fact = *it;

// 	}

// 	if(input != nullptr){

// 	}
// }
