#include <bitset>
#include <map>
#include "../include/hash.h"
#include "../include/flattener.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/var.h"
#include <execution>
#include <thread>

namespace cre {

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

std::vector<FlagGroup> Flattener::default_flags ={
	FlagGroup(HashMap<std::string, Item>(
		{ {{"visible",  Item(true)}} }
	))
};

// std::vector<FlagGroup> default_flags(){
// 	HashMap<std::string, Item> def_hash_map= {
// 		{{"visible",  Item(true)}}
// 	};
// 	std::vector<FlagGroup> flag_groups = {FlagGroup(def_hash_map)};
// 	return flag_groups;
// }

void Flattener_dtor(const CRE_Obj* x) {
	delete ((Flattener* ) x);
}

// Main Constructor
Flattener::Flattener(
		FactSet* _input, 
		const std::vector<FlagGroup>& _flag_groups,
		bool _use_vars,
		bool _add_exist_stubs,
		uint8_t _triple_order) :
	IncrementalProcessor(_input),
	flag_groups(_flag_groups),
	use_vars(_use_vars), 
	add_exist_stubs(_add_exist_stubs), 
	triple_order(_triple_order), 
	subj_ind(_triple_order == TRIPLE_ORDER_SVO ? 0 : 1),
	verb_ind(_triple_order == TRIPLE_ORDER_SVO ? 1 : 0)
	{
	// cout << "CONSTRUCT:" << _input->get_refcount() << endl;
	this->init_control_block(&Flattener_dtor);
}

Flattener::Flattener(FactSet* _input,
	 	  bool _use_vars,
	 	  bool _add_exist_stubs,
	 	  uint8_t _triple_order
) : Flattener(_input, default_flags, _use_vars, _add_exist_stubs, _triple_order)
{}



Flattener::Flattener(FactSet* _input,
			const HashMap<std::string, Item>& target_flags,
			bool _use_vars,
			bool _add_exist_stubs,
		 	uint8_t _triple_order
	 	  
) : Flattener(_input, _format_flags(target_flags), _add_exist_stubs, _triple_order)
{}

Flattener::Flattener(FactSet* _input,
		  const std::vector<HashMap<std::string, Item>>& target_flags,
		bool _use_vars,
	  	bool _add_exist_stubs,
 	  	uint8_t _triple_order
 	  
) : Flattener(_input, _format_flags(target_flags), _add_exist_stubs, _triple_order)
{}


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

	// Explicit const maybe helps branch prediction?
	const bool _use_vars = use_vars; 
	const bool _add_exist_stubs = add_exist_stubs; 

	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;
		size_t L = fact->length;
		auto mbr_inds = this->get_member_inds(fact->type);
		if(mbr_inds != nullptr){
			L = mbr_inds->size();
		}

		if(_use_vars){
			if(_add_exist_stubs){
				_size += SIZEOF_FACT(1);
			}
			_size += SIZEOF_VAR(0); // One base var per fact
			_size += L*(SIZEOF_VAR(1)); // One deref var per member
			_size += L*(SIZEOF_FACT(2)); // One pair per member			
		}else{
			if(_add_exist_stubs){
				_size += SIZEOF_FACT(1);
			}
			_size += L*(SIZEOF_FACT(3));
		}
	}
	return _size;
}


size_t Flattener::_fact_to_var_pairs(
	Fact* __restrict in_fact,
	const std::vector<ref<Var>>& fact_vars
	){

	FactType* __restrict type = in_fact->type;
	auto mbr_inds = this->get_member_inds(type);
	ref<Var> subj_var = fact_vars[in_fact->f_id];
	
	// Make stub like (varname,)
	if(add_exist_stubs){
		ref<Fact> subj_fact = builder.alloc_fact(nullptr, 1);
		subj_fact->set_unsafe(0, subj_var.get());	
		subj_fact->immutable = true;
		builder.fact_set->_declare_back(std::move(subj_fact));

	}

	
	// Make pairs like (varname.attr, value)
	auto make_preds = [&](size_t mbr_ind){
		// size=2, untyped, and immutable, 
		ref<Fact> out_fact = builder.alloc_fact(nullptr, 2);


		ref<Var> verb_var;
		if(type != nullptr && mbr_ind < type->members.size()){
			// int mbr_ind = head_type->get_attr_index(attr);

			DerefInfo* deref = (DerefInfo*) alloca(sizeof(DerefInfo));
			deref->deref_type = ((FactType*) subj_var->head_type)->get_item_type(mbr_ind);
			deref->mbr_ind = mbr_ind;
			deref->deref_kind = DEREF_KIND_ATTR;

			// derefs[0] = DEREF_KIND_ATTR;
		// 	return _extend_unsafe(deref, 1, alloc_buffer);
		// }

			verb_var = subj_var->_extend_unsafe(deref, 1, builder.alloc_buffer);
		}else{
			verb_var = subj_var->extend_item(mbr_ind, builder.alloc_buffer);

		}

		out_fact->set_unsafe(0, verb_var.get());

		// if(type != nullptr && ind < type->members.size()){
		// 	out_fact->set_unsafe(verb_ind /* 1 or 0 */,
		// 	 			type->member_names_as_items[ind]);
		// 	// cout << "FLAT PRED: " << type->member_names_as_items[ind] << uint64_t(type->member_names_as_items[ind].val) << endl;
		// }else{
		// 	out_fact->set(verb_ind /* 1 or 0 */, int(ind));
		// }

		// If 'value' is another fact then (varname.attr, other_varname)
		Item obj_item = in_fact->get(mbr_ind);
		if(obj_item.t_id == T_ID_FACT && obj_item.val != 0){
			Fact* obj_fact = obj_item.as_fact();
			ref<Var> obj_var = fact_vars[obj_fact->f_id];

			out_fact->set_unsafe(1, obj_var.get());	
		}else{
			out_fact->set_unsafe(1, obj_item);		
		}



		
		out_fact->immutable = true;
		builder.fact_set->_declare_back(std::move(out_fact));
		// out_fact->alloc_buffer = builder.alloc_buffer;
		// builder.alloc_buffer->inc_ref();
		// _declare_to_empty(builder.fact_set, out_fact, 3, NULL);

		// cout << uint64_t(out_fact) <<  " OUT FACT: " << out_fact << " L=" << out_fact->length << endl;
	};
	

	if(mbr_inds != nullptr){
		for(auto mbr_ind : *mbr_inds){
			make_preds(mbr_ind);
		}	
	}else{
		for(size_t mbr_ind=0; mbr_ind < in_fact->length; mbr_ind++){
			make_preds(mbr_ind);
		}
	}	
	return 0;	

}


size_t Flattener::_fact_to_wme_triples(Fact* __restrict in_fact){
	FactType* __restrict type = in_fact->type;
	auto mbr_inds = this->get_member_inds(type);
		
	// Make Subject from unique_id or f_id
	auto u_ind = get_unique_id_index(type);
	// cout << "U_IND: " << u_ind << endl;


	Item subj_item = (u_ind == -1 ? 
						Item(in_fact->f_id) :
						in_fact->get(u_ind)
					);

	// Make a new Fact 
	if(add_exist_stubs){
		ref<Fact> subj_fact = builder.alloc_fact(nullptr, 1);
		subj_fact->set_unsafe(0, subj_item);
		subj_fact->immutable = true;
		builder.fact_set->_declare_back(std::move(subj_fact));
		
		// subj_item = Item(subj_fact);
	}
	

	auto make_preds = [&](size_t ind){
		ref<Fact> out_fact = builder.alloc_fact(nullptr, 3);

		out_fact->set_unsafe(subj_ind /* 0 or 1 */, subj_item);

		if(type != nullptr && ind < type->members.size()){
			out_fact->set_unsafe(verb_ind /* 1 or 0 */,
			 			type->member_names_as_items[ind]);
			// cout << "FLAT PRED: " << type->member_names_as_items[ind] << uint64_t(type->member_names_as_items[ind].val) << endl;
		}else{
			out_fact->set(verb_ind /* 1 or 0 */, int(ind));
		}

		Item obj_item = in_fact->get(ind);
		if(obj_item.t_id == T_ID_FACT && obj_item.val != 0){
			Fact* obj_fact = obj_item.as_fact();
			auto u_ind = get_unique_id_index(obj_fact->type);

			obj_item = (u_ind == -1 ? 
						Item(obj_fact->f_id) :
						obj_fact->get(u_ind)
					);
		}

		out_fact->set_unsafe(2, obj_item);	
		out_fact->immutable = true;
		builder.fact_set->_declare_back(std::move(out_fact));
		// out_fact->alloc_buffer = builder.alloc_buffer;
		// builder.alloc_buffer->inc_ref();
		// _declare_to_empty(builder.fact_set, out_fact, 3, NULL);

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


std::vector<ref<Var>> Flattener::_make_fact_vars(FactSet* input){
	std::vector<ref<Var>> fact_vars(input->capacity(), nullptr);

	for (auto it = input->begin(); it != input->end(); ++it) {
		Fact* fact = *it;
		auto u_ind = get_unique_id_index(fact->type);

		Item subj_item = (u_ind == -1 ? 
					Item(fact->f_id) :
					fact->get(u_ind)
				);

		CRE_Type* var_type = fact->type == nullptr ? cre_Fact : fact->type;
		ref<Var> var = new_var(subj_item, var_type, nullptr, 0, builder.alloc_buffer);
		
        fact_vars[fact->f_id] = var;
	}
	return fact_vars;
}

size_t Flattener::_update_init(){
	size_t buffer_size = _calc_buffer_size();
	builder = FactSetBuilder(input->size(), buffer_size);

	

	
	if(use_vars){
		std::vector<ref<Var>> fact_vars = _make_fact_vars(input);
		// for (auto it = fact_vars.begin(); it != fact_vars.end(); ++it) {
		// 	cout << "BEF REFCOUNT:" << (*it)->get_refcount() << endl;
		// }
		for (auto it = input->begin(); it != input->end(); ++it) {
			Fact* fact = *it;
			_fact_to_var_pairs(fact, fact_vars);
		}	
		// for (auto it = fact_vars.begin(); it != fact_vars.end(); ++it) {
		// 	cout << "AFT REFCOUNT:" << (*it)->get_refcount() << endl;
		// }
	}else{
		for (auto it = input->begin(); it != input->end(); ++it) {
			Fact* fact = *it;
			_fact_to_wme_triples(fact);
		}
	}

	// cout << "OUTPUT:" <<  builder.fact_set->get_refcount() << endl;
	
	return 0;
}

ref<FactSet> Flattener::apply(FactSet* fs){
	// cout << "INPUTT:" <<  fs->get_refcount() << endl;
	input = fs;
	_update_init();
	return builder.fact_set;

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

} // NAMESPACE_END(cre)
