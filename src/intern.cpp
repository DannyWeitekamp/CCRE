#include "../include/intern.h"
// #include "../include/unordered_dense.h"

// string s_id_to_string(uint64_t s_id){
// 	auto key_val = bit_cast<pair<string,InternStr*>*>(s_id);
// 	return key_val->first;
// }

// uint32_t intern_refcount(uint64_t s_id){
// 	auto key_val = bit_cast<pair<string,InternStr*>*>(s_id);
// 	return key_val->second->refcount;
// }

// uint32_t intern_refcount(string& s){
// 	auto itr = intern_map.find(s);
// 	if(itr == intern_map.end()){
// 		return 0;
// 	}else{
// 		return itr->second->refcount; 
// 	}
// }
// std::unordered_map<std::string, InternStr*, CREHash, std::equal_to<>> intern_map;
// struct MyEqual : public std::equal_to<>
// {
// 	using is_transparent = void;
// };

// ankerl::unordered_dense::set<std::string_view, CREHash, std::equal_to<>> intern_set = {};
// std::unordered_map<std::string, InternStr*, CREHash, std::equal_to<>> intern_map;
HashSet<std::string_view> intern_set = {};

// std::pair<std::string_view, uint64_t> intern_ret_hash(const std::string_view& sv){
// 	// Note: Optimizer will probably reused hash, should be free.
// 	uint64_t hash = CREHash()(sv);
// 	std::string_view new_sv = intern(sv);
//     return std::make_pair(new_sv, hash);
// }

InternStr::InternStr(const std::string_view& sv, uint64_t _hash) :
	std::string_view(sv), hash(_hash) {
}


InternStr intern(const std::string_view& sv){
	uint64_t hash = CREHash{}(sv);
	auto it = intern_set.find(sv);
    if (it != intern_set.end()) {
        return InternStr(*it, hash);
    }    
    // If not found, create a new string and insert it into the intern set
	string* new_str = new string(sv);
    auto itr = intern_set.insert(*new_str);
	// std::string_view inserted = *itr.first;
	return InternStr(*itr.first, hash);

    // return inserted;
}

// If not found, create a new string and insert it into the intern set


// -- OLD Broken- 
// auto itr = intern_set.find(sv);
// if(itr == intern_set.end()){
// 	auto it = intern_set.insert(std::move(std::string(sv))).first;
// 	return std::string_view(it->data(), it->length());
// }else{
// 	return std::string_view(itr->data(), itr->length());
// }

// -- OLD Bad Alloc- 
// size_t length = sv.length();
    // cout << "length:" << length;
	// char* new_str = static_cast<char*>(malloc(length + 1)); // +1 for null terminator
	// memcpy(new_str, sv.data(), length);
	// new_str[length] = '\0'; // Null-terminate the string
	// auto inserted = intern_set.insert(std::string_view(new_str, length));
	// return *inserted.first;


ostream& operator<<(std::ostream& out, InternStr fs){
	// return out << std::string_view(fs.data, fs.length);
	return out << std::string_view(fs);
}