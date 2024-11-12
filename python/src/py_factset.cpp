#include "../include/py_cre_core.h"
#include "../../include/factset.h"


std::tuple<
	std::vector<std::tuple<nb::handle, FactType*, size_t, size_t>>,
	HashMap<std::string_view, size_t>,
	size_t
>
_collect_fact_infos(nb::handle py_collection){
    auto ref_type_obj = nb::str("type");
    // CRE_Context* context = current_context;

    std::vector<std::tuple<nb::handle, FactType*, size_t, size_t>> fact_infos;
    HashMap<std::string_view, size_t> fact_map = {};

    size_t index = 0;
    size_t byte_offset = 0;

    // Lambda Function
    auto make_fact_info = [&](
    	const std::string_view& fact_id,
    	nb::handle val
    ){
    	// TODO: Handle integer keys
        // if(nb::isinstance<nb::int>(key)):
            // std::to_string()
        
        size_t length; 
        FactType* type = nullptr;

        // cout << "B" << endl;
        if(nb::isinstance<nb::dict>(val)){
            nb::dict fact_dict = nb::cast<nb::dict>(val);
            length = fact_dict.size();
            
            // cout << "C" << endl;

            if(fact_dict.contains(ref_type_obj)){
                // cout << "D" << endl;
                // nb::print(fact_dict[ref_type_obj]);
                type = FactType_from_py(fact_dict[ref_type_obj]);
                // cout << "E" << endl;
                if(type->members.size() > length){
                    length = type->members.size();
                }
                // cout << "F" << endl;
                length -= 1; // Don't count type in the count            
            }
            
            if(type != nullptr){
                length = std::max(length, size_t(type->members.size()));
            }
        }else if(nb::isinstance<nb::list>(val)){
            nb::list fact_list = nb::cast<nb::list>(val);
            length = fact_list.size();
        }else if(nb::isinstance<nb::tuple>(val)){       
            nb::tuple fact_tuple = nb::cast<nb::tuple>(val);
            length = fact_tuple.size();
        }else{
            throw std::runtime_error("Fact item with key " + std::string(fact_id) + " is not a dict.");
        }

        // cout << "C" << endl;
        fact_infos.push_back({val, type, length, byte_offset});
        auto [it, inserted] = fact_map.insert({fact_id, index});
        // cout << fact_id << endl;
        if(!inserted){
            throw std::runtime_error("Duplicate fact identifier: " + std::string(fact_id));
        }
        byte_offset += SIZEOF_FACT(length);//sizeof(Fact) + sizeof(Item) * length;   
    };

    if(nb::isinstance<nb::dict>(py_collection)){
    	nb::dict d = nb::cast<nb::dict>(py_collection);
	    fact_infos.reserve(d.size());
	    fact_map.reserve(d.size());
	    for (auto [key, val] : d) {
	    	std::string_view fact_id = nb::cast<std::string_view>(key);
	        make_fact_info(fact_id, val);
	        index++;
	    }
	}else if(nb::isinstance<nb::list>(py_collection)) {
		nb::list lst = nb::cast<nb::list>(py_collection);
		fact_infos.reserve(lst.size());
	    fact_map.reserve(lst.size());
	    for (auto val : lst) {
	    	std::string fact_id = std::to_string(index);
	    	make_fact_info(fact_id, val);
	        index++;
	    }
	}
	return std::make_tuple(fact_infos, fact_map, byte_offset);
}

Item _resolve_possible_ref(Item item, 
						   CRE_Type* mbr_type,
						   const std::string_view& ref_prefix,
						   HashMap<std::string_view, size_t>& fact_map,
						   const std::vector<std::tuple<nb::handle, FactType*, size_t, size_t>>& fact_infos,
						   const FactSetBuilder& builder 
	){
	if(item.t_id == T_ID_STR){
        std::string_view item_str = item.as_string();

        // Reference to another fact
        size_t ref_len = ref_prefix.size();
        std::string_view ref_str = {};

        // If the prefix is present like "@bob"
        if(item_str.length() > 0 && 
           !item_str.compare(0, ref_len, ref_prefix)
           ){
            ref_str = std::string_view(
            	item_str.data()+ref_len,
            	item_str.length()-ref_len
            );

        // If or if the member's type is FactType
        }else if(mbr_type && 
        		 mbr_type->t_id == T_ID_FACT){
        	ref_str = item_str;
        }

        if(ref_str.size() > 0){
        	auto itr = fact_map.find(ref_str);
        	if(itr == fact_map.end()){
        		throw std::invalid_argument(
        			"Cannot resolve identifier \"" + std::string(item_str) + "\" for Fact reference." 
        		);
        	}
        	size_t index = itr->second;
            size_t offset = std::get<3>(fact_infos[index]);
            Fact* fact_ptr = (Fact*)(builder.alloc_buffer->data + offset);
            item = Item(fact_ptr);
        }
    }
    return item;
}

// template <
// 	// Main Types
// 	typename container_t,
// 	typename dict_t,
// 	typename list_t,
// 	typename obj_t,
// 	typename attr_get_t,

// 	typename is_dict,
// 	typename is_list,

// 	typename to_dict,
// 	typename to_list,
// 	typename to_item,	
// >
// class FactSetTranslator {
//     std::string_view type_attr="type";
//     std::string_view ref_prefix="@";
//     HashMap<std::string_view, size_t> fact_map = {};
//     std::vector<std::tuple<obj_t, FactType*, size_t, size_t>>& fact_infos = {};
//     FactSetBuilder builder;

//     FactSetTranslator(
//     	const std::string_view& type_attr="type", 
// 		const std::string_view& ref_prefix="@") : type_attr(_type_attr), ref_prefix(_ref_prefix){
//     }

//     void _collect_fact_infos(container_t container){

//     }

//     ref<FactSet> translate(container_t container){
//     	fact_infos = {};
//     	fact_map = {};


//     }
// }


static ref<FactSet> _FactSet_from_py(
	nb::handle py_collection,
	const std::string_view& type_attr="type", 
	const std::string_view& ref_prefix="@") {


	auto [fact_infos, fact_map, n_bytes] = _collect_fact_infos(py_collection);

    // cout << "byte_offset:" << byte_offset << endl;
    FactSetBuilder builder = FactSetBuilder(fact_infos.size(), n_bytes);

    // --------------
    // : Second Pass: Building Each object

    for(auto& fact_info : fact_infos){
        auto& fact_obj = std::get<0>(fact_info);
        FactType* type = std::get<1>(fact_info);
        size_t length = std::get<2>(fact_info);
        // size_t offset = std::get<3>(fact_info);

        ref<Fact> fact = builder.add_empty(length, type);
        fact->type = type;

        if(nb::isinstance<nb::dict>(fact_obj)){
            nb::dict fact_dict = nb::cast<nb::dict>(fact_obj);
            for (auto [key, val] : fact_dict){

                std::string_view key_str = nb::cast<std::string_view>(key);
                // Ignore the 'type' member (handled above)
                if(key_str == type_attr){
                    continue;
                }

                // If the key is a number then used it as the index
                char* p;
                int64_t index = strtol(key_str.data(),&p,10);
                if(*p != 0) index = -1;

                // Otherwise get the key's index from the fact type
                if(type != nullptr){
                    index = type->get_attr_index(key_str);
                }

                // Throw error if member index cannot be resolved
                if(index == -1){
                    std::string type_str = type != nullptr ? std::string(type->name) : "NULL";
                    std::string error_msg = "Key '" + std::string(key_str) + "' is not an integer or named member of fact type '" + type_str + "'.";
                    throw std::runtime_error(error_msg);
                }

                Item item = Item_from_py(val);
                item = _resolve_possible_ref(item,
                	index == -1 ? nullptr : type->members[index].type,
                	ref_prefix, fact_map, fact_infos, builder
                );
                
                fact->set(index, item);
            }
            // _init_fact(fact, length, type);
            // fact->alloc_buffer = builder.alloc_buffer;
            builder.fact_set->_declare_back(fact);
        }
        // builder.alloc_buffer->add_ref(fact_infos.size());
    }
    return builder.fact_set;
}   

void init_factset(nb::module_ & m){
	nb::class_<FactSet>(m, "FactSet", nb::type_slots(cre_obj_slots))

	// NOTE: Dumb issue where new_ doesn't take 
	.def(nb::new_([](){
		return ref<FactSet>(new FactSet());
	}), nb::rv_policy::reference)

	.def(nb::new_([](size_t n_facts){
		return ref<FactSet>(new FactSet(n_facts));
	}), "n_facts"_a, nb::rv_policy::reference)

    .def("__str__", &FactSet::to_string, "format"_a="FactSet{{\n  {}\n}}", "delim"_a="\n  ")
    .def("__len__", &FactSet::size)


    .def("__iter__",  [](FactSet& fs) {
            return nb::make_iterator(nb::type<FactSet>(), "iterator",
                                     fs.begin(), fs.end());
        }, nb::keep_alive<0, 1>(), nb::rv_policy::reference
    )

    .def("declare", &FactSet::declare)
    // .def("declare", [](FactSet& self, ref<Fact> fact){
    // 	return self.declare(fact);
    // })
    .def_static("from_py", &_FactSet_from_py,
     	"obj"_a, "type_attr"_a="type", "ref_prefix"_a="@",  nb::rv_policy::reference)
    .def_static("from_json", [](nb::str json)->ref<FactSet>{
        std::string json_str = nb::cast<std::string>(json);
        return FactSet::from_json(json_str.data(), json_str.size());
        }, nb::rv_policy::reference)
    .def_static("from_json_file", &FactSet::from_json_file, nb::rv_policy::reference)
    .def_static("to_json", &FactSet::to_json)
    ;
}
