#include "../include/types.h"
#include "../include/context.h"

using std::cout;
using std::endl;

// CRE_Context::_add_type implementation
size_t CRE_Context::_add_type(CRE_Type* t) {
    size_t index;
    if (type_name_map.find(t->name) == type_name_map.end()) {
        // cout << "Adding type: " << t->name << endl;
        index = types.size();
        types.push_back(t);
        // std::string* s = new std::string(t->name);
        type_name_map[t->name] = index;
    } else {
        // cout << "Replacing type: " << t->name << endl;
        index = type_name_map[t->name];
        types[index] = t;
    }    
    t->context = this;
    t->type_index = index;
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
    ensure_builtins();
    // cout << "CRE_Context constructor " << cre_builtins.size() << endl;
    // cout << "WHATEVER";
    // for (auto t : cre_builtins) {
    //     cout << (uint64_t) t << endl;
    //     // _add_type((CRE_Type*) t);
    // }
    for (auto t : cre_builtins) {
        // cout << t->name << endl;
        _add_type((CRE_Type*) t);
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


extern "C" CRE_Context* CRE_set_current_context(CRE_Context* context) {
    CRE_Context* old_context = current_context;
    current_context = context;
    return old_context;
}

CRE_Context* default_context = new CRE_Context("default");
CRE_Context* current_context = default_context;
