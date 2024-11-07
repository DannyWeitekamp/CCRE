#include "../include/py_cre_core.h"
#include "../../include/factset.h"


static ref<FactSet> _FactSet_from_dict(nb::dict d) {
    // Globals
    // cout << "START" << endl;
    auto ref_type_obj = nb::str("type");
    CRE_Context* context = current_context;

    std::vector<std::tuple<nb::handle, FactType*, size_t, size_t>> fact_infos;
    HashMap<std::string_view, size_t> fact_map = {};

    // cout << "A" << endl;

    fact_infos.reserve(d.size());
    fact_map.reserve(d.size());
    size_t byte_offset = 0;
    size_t index = 0;
    for (auto [key, val] : d) {

        // TODO: Handle integer keys
        // if(nb::isinstance<nb::int>(key)):
            // std::to_string()
        std::string_view fact_id = nb::cast<std::string_view>(key);
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
        index++;
    }

    // cout << "byte_offset:" << byte_offset << endl;
    FactSetBuilder builder = FactSetBuilder(d.size(), byte_offset);

    // --------------
    // : Second Pass: Building Each object

    for(auto& fact_info : fact_infos){
        auto& fact_obj = std::get<0>(fact_info);
        FactType* type = std::get<1>(fact_info);
        size_t length = std::get<2>(fact_info);
        // size_t offset = std::get<3>(fact_info);

        Fact* __restrict fact = builder.next_empty(length);
        fact->type = type;

        if(nb::isinstance<nb::dict>(fact_obj)){
            nb::dict fact_dict = nb::cast<nb::dict>(fact_obj);
            for (auto [key, val] : fact_dict){

                std::string_view key_str = nb::cast<std::string_view>(key);
                // Ignore the 'type' member (handled above)
                if(key_str == "type"){
                    continue;
                }

                // If the key is a number then used it as the index
                char* p;
                int64_t index = strtol (key_str.data(),&p,10);
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
                if(item.t_id == T_ID_STR){
                    std::string_view item_str = item.as_string();

                    // Reference to another fact
                    if(item_str.length() > 0 && item_str[0] == '@'){
                        std::string_view ref_str = std::string_view(item_str.data()+1, item_str.length()-1);
                        size_t index = fact_map[ref_str];
                        size_t offset = std::get<3>(fact_infos[index]);
                        Fact* fact_ptr = (Fact*)(builder.alloc_buffer->data + offset);
                        item = Item(fact_ptr);
                    }
                }
                fact->set(index, item);
            }
            _init_fact(fact, length, type);
            builder.fact_set->_declare_back(fact);
        }
    }
    return builder.fact_set;
}   

void init_factset(nb::module_ & m){
	nb::class_<FactSet>(m, "FactSet")
    .def("__str__", &FactSet::to_string, "format"_a="FactSet{{\n  {}\n}}", "delim"_a="\n  ")
    .def("__len__", &FactSet::size)
    .def_static("from_dict", &_FactSet_from_dict, nb::rv_policy::reference)
    .def_static("from_json", [](nb::str json)->ref<FactSet>{
        std::string json_str = nb::cast<std::string>(json);
        return FactSet::from_json(json_str.data(), json_str.size());
        }, nb::rv_policy::reference)
    .def_static("from_json_file", &FactSet::from_json_file, nb::rv_policy::reference)
    .def_static("to_json", &FactSet::to_json)
    ;
}
