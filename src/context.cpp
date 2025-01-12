#include "../include/types.h"
#include "../include/context.h"

using std::cout;
using std::endl;

namespace cre {

// CRE_Context::_add_type implementation
size_t CRE_Context::_add_type(CRE_Type* t) {
    t->inc_ref();

    size_t index;
    auto itr = type_name_map.find(t->name);
    if (itr == type_name_map.end()) {
        // cout << "Adding type: " << t->name << endl;
        index = types.size();
        types.push_back(t);
        // std::string* s = new std::string(t->name);
        type_name_map[t->name] = index;
    } else {
        // cout << "Replacing type: " << t->name << endl;

        // When a type is replaced push it to overwritten_types 
        overwritten_types.push_back(types[itr->second]);
        index = type_name_map[t->name];
        types[index] = t;
    }    
    t->context = this;
    t->type_index = index;

    // cout << t->name << "(a):" << t->get_refcount() << endl; 
    // t->t_id = t_id;
    return index;
};

FactType* CRE_Context::_get_fact_type(const std::string_view& name) noexcept {
	CRE_Type* ct = this->_get_type(name);	
	if(ct == nullptr || ct->builtin){
		return nullptr;
	}
	return (FactType*) ct;
}

FactType* CRE_Context::get_fact_type(const std::string_view& name) {
    FactType* fact_type = this->_get_fact_type(name);
    if(fact_type == nullptr){
        throw std::runtime_error("Fact type '" + std::string(name) + "' not defined in CRE_Context: " + this->name);    
    }
    return fact_type;
}

CRE_Type* CRE_Context::_get_type(const std::string_view& name) noexcept{
	auto itr = type_name_map.find(name);
	if (itr != type_name_map.end()) {
        uint16_t type_ind = itr->second;
        return types[type_ind];
    }else{
    	return nullptr;
    }
}

CRE_Type* CRE_Context::get_type(const std::string_view& name) {
    CRE_Type* type = this->_get_type(name);
    if(type == nullptr){
        throw std::runtime_error("Type '" + std::string(name) + "' not defined in CRE_Context: " + this->name);    
    }
    return type;
}



// CRE_Context constructor implementation
CRE_Context::CRE_Context(std::string _name) : name(_name) {
    if(_name == "default"){
        cre_builtins = make_builtins();    
    }else{
        cre_builtins = default_context->cre_builtins;
    }
    
    // cout << "CRE_Context constructor " << cre_builtins.size() << endl;
    // cout << "WHATEVER";
    // for (auto t : cre_builtins) {
    //     cout << (uint64_t) t << endl;
    //     // _add_type((CRE_Type*) t);
    // }
    for (auto t : cre_builtins) {
        
        _add_type((CRE_Type*) t);
        // cout << t->name << "(a):" << t->get_refcount() << endl; 
    }
};

std::string CRE_Context::to_string() {
    std::stringstream ss;
    ss << "CRE_Context: " << name << endl;
    for(auto t : types){
        ss << "  " << t->name << endl;
    }
    return ss.str();
}


CRE_Context::~CRE_Context(){
    for(auto type : types){
        // cout << type->name << "(d):" << type->get_refcount() << endl; 
        type->dec_ref();
    }
    for(auto type : overwritten_types){
        type->dec_ref();   
    }
    for(auto sv : intern_set){
        free((void*) sv.data());
    }
}


CRE_Context* get_context(std::string_view name) {
    auto it = __all_CRE_contexts.find(name);
    if(it != __all_CRE_contexts.end()){
        return (it->second.get());
    }else{
        throw std::runtime_error("No such context \"" + std::string(name) + "\"");
    }
}

CRE_Context* set_current_context(CRE_Context* context) {
    CRE_Context* old_context = current_context;
    current_context = context;
    return old_context;
}

CRE_Context* set_current_context(std::string_view name) {
    CRE_Context* context = nullptr;
    auto it = __all_CRE_contexts.find(name);
    if(it == __all_CRE_contexts.end()){
        std::string _name = std::string(name);
        auto unq_context = std::make_unique<CRE_Context>(_name);
        auto [itr,_] = __all_CRE_contexts.insert({std::move(_name), std::move(unq_context)});
        context = (itr->second.get());
    }else{
        context = (it->second.get());
    }
    return set_current_context(context);
}

HashMap<std::string, std::unique_ptr<CRE_Context>> __all_CRE_contexts = {};
CRE_Context* current_context = nullptr;

// Special lambda initializer before 
CRE_Context* default_context = []()
{   
    set_current_context("default");
    return get_context("default");
}();

} // NAMESPACE_END(cre)
