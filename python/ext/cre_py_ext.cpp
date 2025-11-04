#include "../include/py_cre_core.h"

// #include <iostream>
// #include <string>
// #include <atomic>
// #include <memory> 

// // Note: these need to included in this order
// #include <nanobind/nanobind.h>
// #include "../../include/cre_obj.h"
// #include "../../include/ref.h"
// // 

// #include "../../include/context.h"
// #include "../../include/types.h"
// #include "../../include/intern.h"
// #include "../../include/fact.h"
// #include "../../include/factset.h"
// #include "../../include/flattener.h"
// #include "../../include/vectorizer.h"

// #include <nanobind/stl/string.h>
// #include <nanobind/stl/string_view.h>
// #include <nanobind/ndarray.h>

// namespace nb = nanobind;
// using namespace nb::literals;
// using std::cout;
// using std::endl;


// // ---------------------------------------------------------------
// // : Deallocator For decref()'ing on Garbage Collect  


// typedef void (*dealloc_ty)(PyObject *self);
// dealloc_ty nb_inst_dealloc = nullptr;

// static void cre_obj_dealloc(PyObject *self){
//     auto self_handle = nb::handle(self);
//     CRE_Obj* cpp_self = (CRE_Obj*) nb::inst_ptr<CRE_Obj>(self_handle);


//     // nb::print("DIED: ", self_handle);
//     // cout << "~Before Die~:" << cpp_self->get_refcount() << endl;    

//     if(nb_inst_dealloc){
//         (*nb_inst_dealloc)(self);
//     }
//     // When the Python proxy object is collected remove 
//     //   the C++ object's reference to it so it isn't reused
//     cpp_self->proxy_obj = nullptr;
//     CRE_decref(cpp_self);//->dec_ref();
// }

// // PyObject *myclass_tp_add(PyObject *a, PyObject *b) {
// //     return PyNumber_Multiply(a, b);
// // }

// PyType_Slot slots[] = {
//     { Py_tp_dealloc, (void *) cre_obj_dealloc },
//     { 0, nullptr }
// };

// class CREDummy : public CRE_Obj{
//     int a;
// };

// // ------------------------------------------------------------------
// // : Nanobind Module



// Item Item_from_py(nb::handle py_obj){
//     if (nb::isinstance<nb::bool_>(py_obj)) {
//         return Item(nb::cast<bool>(py_obj));
//     } else if (nb::isinstance<nb::int_>(py_obj)) {
//         return Item(nb::cast<int>(py_obj));
//     } else if (nb::isinstance<nb::float_>(py_obj)) {
//         return Item(nb::cast<double>(py_obj));
//     } else if (nb::isinstance<nb::str>(py_obj)) {
//         InternStr intern_str = intern(nb::cast<std::string_view>(py_obj));
//         // cout << "Interned: " << intern_str << endl;
//         return Item(intern_str);
    
//     } else {
//         throw std::runtime_error("Item cannot be created from: " + nb::cast<std::string>(nb::str(py_obj)));
//     }
// }

// std::string_view Type_name_from_py(nb::handle py_obj){
//     // nb:print("Type name from py: ", py_obj);
//     std::string_view type_name = "";
//     if(nb::isinstance<nb::str>(py_obj)){
//         type_name = nb::cast<std::string_view>(py_obj);
//     }else if(py_obj.is_type() && !py_obj.is_none()){
//         type_name = nb::cast<std::string_view>(nb::getattr(py_obj, "__name__"));
//     }else{
//         throw std::runtime_error("Expecting string or type object to resolve type name, but got: \"" + nb::cast<std::string>(nb::str(py_obj)) + "\"");
//     }
//     return type_name;
// }

// FactType* FactType_from_py(nb::handle py_obj){
//     std::string_view type_name = Type_name_from_py(py_obj);
//     return current_context->get_fact_type(type_name); // throws if not found
// }

// CRE_Type* Type_from_py(nb::handle py_obj){
//     std::string_view type_name = Type_name_from_py(py_obj);
//     return current_context->get_type(type_name); // throws if not found
// }

// FactType* py_define_fact(nb::str py_name, nb::dict member_infos){
//     // cout << current_context->to_string() << endl;

//     FactType* type = nullptr;

//     std::vector<MemberSpec> members = {};
//     for (auto [py_attr_name, val] : member_infos) {
//         nb::handle type_handle = val;
//         std::optional<nb::dict> py_flags;
        
//         if(nb::isinstance<nb::tuple>(val)){
//             auto tuple = nb::cast<nb::tuple>(val);
//             if(tuple.size() >= 1){
//                 type_handle = tuple[0];
//             }
//             if(tuple.size() >= 2){
//                 py_flags = nb::cast<nb::dict>(tuple[1]);
//             }
//         }

//         CRE_Type* mbr_type = nullptr;
//         // if(nb::isinstance<nb::str>(type_handle)){
//             // auto type_name = Type_from_py(type_handle);//::cast<std::string>(type_handle);
//         mbr_type = Type_from_py(type_handle);//current_context->get_type(type_name);
//         if(mbr_type == nullptr){
//             throw std::runtime_error("Fact type not found.");
//         }
//         // }

//         HashMap<std::string, Item> flags = {};
//         if(py_flags){
//             nb::dict _py_flags = *py_flags;
//             for (auto [py_flag_name, py_flag_val] : _py_flags) {
//                 flags[nb::cast<std::string>(py_flag_name)] = Item_from_py(py_flag_val);
//             }
//         }

//         std::string attr_name = nb::cast<std::string>(py_attr_name);
//         MemberSpec member_spec = py_flags ? 
//             MemberSpec(attr_name, mbr_type, flags) :
//             MemberSpec(attr_name, mbr_type);
//         members.push_back(member_spec);
//     }
//     std::string name = nb::cast<std::string>(py_name);
//     return define_fact(name, members);
// }


// ref<Fact> _py_new_fact(FactType* type, 
//                 int n_pos_args,
//                 nb::detail::fast_iterator args_start,
//                 nb::detail::fast_iterator args_end,
//                 nb::kwargs kwargs){
//     if(type == nullptr && kwargs.size() > 0){
//         throw std::invalid_argument("Keyword argument used in untyped Fact initialization.");
//     }

//     int max_args = n_pos_args + kwargs.size();
//     int items_len = n_pos_args;
//     Item* items = (Item*) alloca(sizeof(Item) * max_args);
//     std::fill(items, items+max_args, Item());

//     auto it = args_start;
//     for (int i=0; it != args_end; ++it, i++) {
//         items[i] = Item_from_py(*it);
//         // nb::print(nb::str("Positional: {}").format(*it));
//     }

//     for (auto [key, val] : kwargs) {
//         std::string_view key_view = nb::cast<std::string_view>(key);
//         int mbr_ind = type->get_attr_index(key_view);
//         if(mbr_ind == -1){
//             throw std::invalid_argument("FactType: \"" + type->name + "\" has no member: \"" + std::string(key_view) + "\"");
//         }
//         if(mbr_ind < n_pos_args){
//             throw std::invalid_argument("Keyword argument \"" + std::string(key_view) + "\" overwrites positional argument " + std::to_string(mbr_ind));
//         }
//         items[mbr_ind] = Item_from_py(val);
//         items_len = std::max(items_len, mbr_ind+1);
//     }

//     Fact* fact = new_fact(type, items, items_len);
//     ref<Fact> fact_ref = ref<Fact>(fact);
//     return fact_ref;
// }

// ref<Fact> NewFact(nb::args args, nb::kwargs kwargs) {
//     FactType* fact_type = nullptr;
//     auto it = args.begin();
//     if(args.size() >= 1){
//         CRE_Context* context = current_context;
//         auto arg0 = *it;
        
//         if(!arg0.is_none()){ 
//             fact_type = FactType_from_py(arg0);
//         }
//         // nb::print(nb::str("args[0]: {}").format(arg0));
//         it++;
//     }
//     return _py_new_fact(fact_type, args.size()-1, it, args.end(), kwargs);  
// }


// static ref<FactSet> _FactSet_from_dict(nb::dict d) {
//     // Globals
//     // cout << "START" << endl;
//     auto ref_type_obj = nb::str("type");
//     CRE_Context* context = current_context;

//     std::vector<std::tuple<nb::handle, FactType*, size_t, size_t>> fact_infos;
//     HashMap<std::string_view, size_t> fact_map = {};

//     // cout << "A" << endl;

//     fact_infos.reserve(d.size());
//     fact_map.reserve(d.size());
//     size_t byte_offset = 0;
//     size_t index = 0;
//     for (auto [key, val] : d) {

//         // TODO: Handle integer keys
//         // if(nb::isinstance<nb::int>(key)):
//             // std::to_string()
//         std::string_view fact_id = nb::cast<std::string_view>(key);
//         size_t length; 
//         FactType* type = nullptr;

//         // cout << "B" << endl;
//         if(nb::isinstance<nb::dict>(val)){
//             nb::dict fact_dict = nb::cast<nb::dict>(val);
//             length = fact_dict.size();
            
//             // cout << "C" << endl;

//             if(fact_dict.contains(ref_type_obj)){
//                 // cout << "D" << endl;
//                 // nb::print(fact_dict[ref_type_obj]);
//                 type = FactType_from_py(fact_dict[ref_type_obj]);
//                 // cout << "E" << endl;
//                 if(type->members.size() > length){
//                     length = type->members.size();
//                 }
//                 // cout << "F" << endl;
//                 length -= 1; // Don't count type in the count            
//             }
            
//             if(type != nullptr){
//                 length = std::max(length, size_t(type->members.size()));
//             }
//         }else if(nb::isinstance<nb::list>(val)){
//             nb::list fact_list = nb::cast<nb::list>(val);
//             length = fact_list.size();
//         }else if(nb::isinstance<nb::tuple>(val)){       
//             nb::tuple fact_tuple = nb::cast<nb::tuple>(val);
//             length = fact_tuple.size();
//         }else{
//             throw std::runtime_error("Fact item with key " + std::string(fact_id) + " is not a dict.");
//         }

//         // cout << "C" << endl;
//         fact_infos.push_back({val, type, length, byte_offset});
//         auto [it, inserted] = fact_map.insert({fact_id, index});
//         // cout << fact_id << endl;
//         if(!inserted){
//             throw std::runtime_error("Duplicate fact identifier: " + std::string(fact_id));
//         }
//         byte_offset += SIZEOF_FACT(length);//sizeof(Fact) + sizeof(Item) * length;
//         index++;
//     }

//     // cout << "byte_offset:" << byte_offset << endl;
//     FactSetBuilder builder = FactSetBuilder(d.size(), byte_offset);

//     // --------------
//     // : Second Pass: Building Each object

//     for(auto& fact_info : fact_infos){
//         auto& fact_obj = std::get<0>(fact_info);
//         FactType* type = std::get<1>(fact_info);
//         size_t length = std::get<2>(fact_info);
//         // size_t offset = std::get<3>(fact_info);

//         Fact* __restrict fact = builder.next_empty(length);
//         fact->type = type;

//         if(nb::isinstance<nb::dict>(fact_obj)){
//             nb::dict fact_dict = nb::cast<nb::dict>(fact_obj);
//             for (auto [key, val] : fact_dict){

//                 std::string_view key_str = nb::cast<std::string_view>(key);
//                 // Ignore the 'type' member (handled above)
//                 if(key_str == "type"){
//                     continue;
//                 }

//                 // If the key is a number then used it as the index
//                 char* p;
//                 int64_t index = strtol (key_str.data(),&p,10);
//                 if(*p != 0) index = -1;

//                 // Otherwise get the key's index from the fact type
//                 if(type != nullptr){
//                     index = type->get_attr_index(key_str);
//                 }

//                 // Throw error if member index cannot be resolved
//                 if(index == -1){
//                     std::string type_str = type != nullptr ? std::string(type->name) : "NULL";
//                     std::string error_msg = "Key '" + std::string(key_str) + "' is not an integer or named member of fact type '" + type_str + "'.";
//                     throw std::runtime_error(error_msg);
//                 }

//                 Item item = Item_from_py(val);
//                 if(item.t_id == T_ID_STR){
//                     std::string_view item_str = item.as_string();

//                     // Reference to another fact
//                     if(item_str.length() > 0 && item_str[0] == '@'){
//                         std::string_view ref_str = std::string_view(item_str.data()+1, item_str.length()-1);
//                         size_t index = fact_map[ref_str];
//                         size_t offset = std::get<3>(fact_infos[index]);
//                         Fact* fact_ptr = (Fact*)(builder.alloc_buffer->data + offset);
//                         item = Item(fact_ptr);
//                     }
//                 }
//                 fact->set(index, item);
//             }
//             _init_fact(fact, length, type);
//             builder.fact_set->_declare_back(fact);
//         }
//     }
//     return builder.fact_set;
// }   

// FlagGroup dict_to_flag_group(nb::dict py_dict){
//     FlagGroup flag_group = {};
//     for (auto [key, val] : py_dict) {
//         flag_group.assign_flag(nb::cast<std::string>(key), Item_from_py(val));
//     }
//     return flag_group;
// }

// std::vector<FlagGroup> py_to_flag_groups(nb::handle py_obj){
//     std::vector<FlagGroup> flags = {};
//     if(nb::isinstance<nb::dict>(py_obj)){
//         nb::dict py_dict = nb::cast<nb::dict>(py_obj);
//         flags = {dict_to_flag_group(py_dict)};    
//     }else if(nb::isinstance<nb::list>(py_obj)){
//         nb::list py_list = nb::cast<nb::list>(py_obj);
//         flags.reserve(py_list.size());
//         for(auto py_flag_group : py_list){
//             if(nb::isinstance<nb::dict>(py_flag_group)){
//                 flags.push_back(dict_to_flag_group(nb::cast<nb::dict>(py_flag_group)));
//             }else{
//                 throw std::runtime_error("Expecting a dict in flag group list, but got: " + nb::cast<std::string>(nb::str(py_flag_group)));
//             }
//         }
//     }else{
//         throw std::runtime_error("Expecting a list of dicts or single dict for flag groups, but got: " + nb::cast<std::string>(nb::str(py_obj)));
//     }
//     return flags;
// }


// // ref<Flattener> py_new_flattener(FactSet* input, nb::handle flag_groups, bool subj_as_fact, int triple_order){
// //     cout << "NEW FLATTENER" << endl;
// //     std::vector<FlagGroup> flag_groups_vec;
// //     if(flag_groups.is_none()){
// //         flag_groups_vec = Flattener::default_flags;
// //     }else{
// //         flag_groups_vec = py_to_flag_groups(flag_groups);
// //     }
// //     // return ref<Flattener>(new Flattener(input, flag_groups_vec, subj_as_fact, triple_order));
// //     ref<Flattener> out_ref = ref<Flattener>(new Flattener(input, flag_groups_vec, subj_as_fact, triple_order));
// //     return out_ref;
// //     // nb::cast<nb::handle>(out_ref);
// //     // auto caster = ;
// //     // nb::handle py_out = nb::detail::make_caster<Flattener>::from_cpp(out_ref.get(), nb::rv_policy::reference, nullptr );
// //     // auto _is = inst_state(py_out);
// //     // std::cout << "!- " << nb::cast<std::string>(nb::str(py_out)) << " STATE: ready=" <<  _is.first << ", destruct=" <<  _is.second << std::endl;
// //     // return py_out;
// //     // return nb::cast<nb::handle>(out_ref);
// //     // return out_ref;
// // }

// using uint_arr = nb::ndarray<uint64_t, nb::numpy, nb::ndim<1>>;
// using double_arr = nb::ndarray<double, nb::numpy, nb::ndim<1>>;

// // std::tuple<uint_arr, double_arr> 
// nb::tuple
//     py_Vectorizer_apply(Vectorizer& self, FactSet* input){
//     auto [nom, cont] = self.apply(input);

//     auto nom_arr = uint_arr
//                     (nom.data(), {nom.size()});
//     auto cont_arr = double_arr
//                     (cont.data(), {cont.size()});
//     return nb::make_tuple(nom_arr, cont_arr);
// }


// Forward Declare various modules
void init_core(nb::module_ & m);
void init_fact(nb::module_ & m);
void init_factset(nb::module_ & m);
void init_func(nb::module_ & m);
void init_logical(nb::module_ & m);
void init_flattener(nb::module_ & m);
void init_vectorizer(nb::module_ & m);


NB_MODULE(cre, m) {
    init_core(m);
    init_fact(m);
    init_factset(m);
    init_func(m);
    init_logical(m);
    init_flattener(m);
    init_vectorizer(m);


    // // Define a dummy Type and use it to set the address of nb_inst_dealloc
    // nb::class_<CREDummy>(m, "CREDummy");
    // auto dum_ty_handle = nb::type<CREDummy>();
    // if (dum_ty_handle.is_valid()) {
    //     PyTypeObject* obj_ty = (PyTypeObject*) dum_ty_handle.ptr();
    //     nb_inst_dealloc = obj_ty->tp_dealloc;
    // }

	// // A very minimal wrapper for a CRE_obj Python side 
	// nb::class_<CRE_Obj>(m, "CRE_Obj")
    // .def("get_refcount", &CRE_Obj::get_refcount)
    // ;

    // nb::class_<CRE_Type>(m, "CRE_Type");

    // nb::class_<FactType>(m, "Fact_Type")
    // // Constructor MyFactType(...)
    // .def("__call__", [](FactType& self, nb::args args, nb::kwargs kwargs){
    //     return _py_new_fact(&self, args.size(), args.begin(), args.end(), kwargs);
    // })
    // // .def(nb::new_(&NewFact), nb::rv_policy::reference)
    // ;

    // // Fact
    // nb::class_<Item>(m, "Item");

    // // Fact
    // nb::class_<Fact>(m, "Fact")
    // .def(nb::new_(&NewFact), nb::rv_policy::reference)
    // .def("__str__", &Fact::to_string, "verbosity"_a=2)
    // .def("__len__", &Fact::size)
    // .def("get_refcount", &Fact::get_refcount)
    // ;

    // nb::class_<FactSet>(m, "FactSet")
    // .def("__str__", &FactSet::to_string, "format"_a="FactSet{{\n  {}\n}}", "delim"_a="\n  ")
    // .def("__len__", &FactSet::size)
    // .def_static("from_dict", &_FactSet_from_dict, nb::rv_policy::reference)
    // .def_static("from_json", [](nb::str json)->ref<FactSet>{
    //     std::string json_str = nb::cast<std::string>(json);
    //     return FactSet::from_json(json_str.data(), json_str.size());
    //     }, nb::rv_policy::reference)
    // .def_static("from_json_file", &FactSet::from_json_file, nb::rv_policy::reference)
    // .def_static("to_json", &FactSet::to_json)
    // ;

    // nb::class_<Flattener>(m, "Flattener")
    // // No inputs constructor (not sure why but this is necessary)
    // .def(nb::new_(
    //     [](){
    //         return ref<Flattener>(new Flattener());
    //         }),
    //     nb::rv_policy::reference)

    // // Constructor w/ inputs 
    // .def(nb::new_([](FactSet* input, nb::handle flag_groups, bool subj_as_fact, int triple_order)->ref<Flattener>{
    //         std::vector<FlagGroup> flag_groups_vec;
    //         if(flag_groups.is_none()){
    //             flag_groups_vec = Flattener::default_flags;
    //         }else{
    //             flag_groups_vec = py_to_flag_groups(flag_groups);
    //         }
    //         // return ref<Flattener>(new Flattener(input, flag_groups_vec, subj_as_fact, triple_order));
    //         return ref<Flattener>(new Flattener(input, flag_groups_vec, subj_as_fact, triple_order));
    //         }),
    //     "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "subj_as_fact"_a=false, "triple_order"_a=TRIPLE_ORDER_SPO,              
    //     nb::rv_policy::reference)
    // // .def(nb::new_(&py_new_flattener),
    // //     "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "subj_as_fact"_a=false, "triple_order"_a=TRIPLE_ORDER_SPO, 
    // //     nb::rv_policy::reference)
    // .def("apply", &Flattener::apply,
    //     // [](Flattener& self, FactSet* fs){
    //     //     return self.apply(fs);
    //     // },
    //     nb::rv_policy::reference)
    // ;

    // nb::class_<Vectorizer>(m, "Vectorizer")
    // .def(nb::new_(
    //     [](){
    //         return ref<Vectorizer>(new Vectorizer());
    //         }),
    //     nb::rv_policy::reference)

    // .def(nb::new_([](uint64_t max_heads, bool one_hot_nominal, bool encode_missings)->ref<Vectorizer>{
    //         return ref<Vectorizer>(new Vectorizer(max_heads, one_hot_nominal, encode_missings));
    //     }), "max_heads"_a=0, "one_hot_nominal"_a=true, "encode_missings"_a=false,
    //     nb::rv_policy::reference)
    // .def("apply", &py_Vectorizer_apply, 
    //     nb::rv_policy::reference)
    // ;

    // m.def("NewFact", &NewFact, nb::rv_policy::reference);
    // m.def("define_fact", &py_define_fact, nb::rv_policy::reference);

    

}
