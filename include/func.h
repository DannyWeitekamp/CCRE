#pragma once

#include <iostream>
#include <map>
#include "../include/ref.h"
#include "../include/types.h"
#include "../include/cre_obj.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/var.h"
#include "../include/item.h"
#include "../include/member.h"

namespace cre {

// Forward Declarations 
struct Func;
struct FuncRef;




// ARGINFO type enums
const uint8_t ARGINFO_CONST = 1;
const uint8_t ARGINFO_VAR = 2;
const uint8_t ARGINFO_FUNC = 3;
const uint8_t ARGINFO_FUNC_UNEXPANDED = 4;

// ------------------------------
// STATUS enums
const uint8_t CFSTATUS_FALSEY = 0;
const uint8_t CFSTATUS_TRUTHY = 1;
const uint8_t CFSTATUS_NULL_DEREF = 2;
const uint8_t CFSTATUS_ERROR = 3;

// ------------------------------
// REFKIND enums
const uint8_t REFKIND_PRIMATIVE = 1;
const uint8_t REFKIND_UNICODE = 2;
const uint8_t REFKIND_STRUCTREF = 3;


// Struct for each argument to a CREFunc which might be assigned 
//  to a primative constant, Var, or other CREFunc.
struct ArgInfo {
	CRE_Type* type;
	// Item* ptr;
	uint16_t kind;
	// uint16_t t_id;
	// uint16_t byte_width;


	// ** Begin Composition-Specific Info ** // 

	// The offset in an temporary stack where intermediate values go 
	uint16_t offset = 0;

	// Base var index 
	uint16_t var_ind = 0;
	uint16_t head_ind = 0;



	// The offset of the base source; either a const written into the Func or 
	//   the input arguments. 
	// uint16_t base_offset; 


	bool has_deref;
	


	ArgInfo(CRE_Type* _type, //Item* _ptr,
	 		uint16_t _kind, 
	 		uint16_t _offset, 
	 		uint16_t _var_ind, 
	 		uint16_t _head_ind,
	 		// uint16_t _base_offset, 
	 		bool _has_deref) :
		 type(_type), //ptr(_ptr),
		 kind(_kind), 
		 offset(_offset), 
		 var_ind(_var_ind),
		 head_ind(_head_ind),
		// base_offset(_base_offset),
		 has_deref(_has_deref)
	{};
};

// Struct w/ instructions for executing a child CREFunc that needs to be 
//  run before the main root CREFunc in a composition of CREFuncs
struct InstrType {
	// The ptr to the CREFunc
    Func* cf_ptr;

    // Data ptr to copy the CREFunc's return value into.
    Item* return_data_ptr;
    // ('return_data_ptr', np.int64), 

    // // The byte-width of the return value  
    // ('size',np.uint32), 

   	// // 1 for unicode_type 2 for other. Needed because unicode_strings 
    // //  don't have meminfo ptrs as first member.
    // ('ref_kind',np.uint32)
};

struct HeadInfo {
	// The pointer to the CREFunc for which this is a head argument.
	Func* cf_ptr = nullptr;

	// Enum for CONST, VAR, FUNC, FUNC_UNEXPANDED.
	uint8_t kind = ARGINFO_VAR;

	// If the Var associated with this has a dereference chain.
	bool has_deref = false;

	// The index of the root argument this head corresponds to.
	uint16_t arg_ind = uint16_t(-1); 

	// base value's type.
	CRE_Type* base_type; 

	// head value's type.
	CRE_Type* head_type; 

	// The pointer to the Var for this head. Its ref is held in "ref{i}" member.
	Var* var_ptr = nullptr;

	// Data pointer for the a{i} member for the base.
	Item* arg_data_ptr;

	// Data pointer for the h{i} member for the resolved head.
	Item* head_data_ptr;

	// 1 for unicode_type 2 for other. Needed because unicode_strings don't 
    // have meminfo ptrs as first member so need different inc/decref implmentation.
  uint32_t ref_kind;

	// The byte-width of the head value.
	// uint32_t byte_width;

	// HeadInfo()       				   = default;
	// HeadInfo(const HeadInfo& hi)       = default;
	// HeadInfo(HeadInfo&& hi)            = default;
    // HeadInfo& operator=(HeadInfo&& hi) = default;

};

// The span within the Func's head_infos vector for a particular base Var
struct HeadRange {
	uint16_t start;
	uint16_t end;
	HeadRange(uint16_t _start, uint16_t _end) :
	 	start(_start), end(_end) {};
};


struct OriginData {
	Func* func;
	std::string name;
	std::string expr_template;
	std::string shorthand_template;
	// std::vector<void *> repr_const_addrs;
};

template <typename T>
std::string get_type_name() {
    #ifdef __GNUC__
        std::string s = __PRETTY_FUNCTION__;
        // Extract the type name from the pretty function string (compiler-dependent parsing)
        size_t start = s.find(" = ") + 3;
        size_t end = s.find("&", start);
        return s.substr(start, end - start);
    #elif _MSC_VER
        std::string s = __FUNCSIG__;
        // Extract the type name from the function signature string (compiler-dependent parsing)
        size_t start = s.find("get_type_name<") + 14;
        size_t end = s.find(">(void)", start);
        return s.substr(start, end - start);
    #else
        return "Unknown Type (Compiler specific)";
    #endif
}

template <typename T>
inline void* _copy_convert_from_str(void* dest, const T& val, CRE_Type* type){
	switch(type->t_id) {
		case T_ID_STR:
  	{
  		// std::string_view* s = std::string_view(data, length);
  		new (dest) StrBlock(val); // Construct string_view in buffer
  		break;
  	}
  	// case T_ID_INT:
  	// 	*((int64_t*) dest) = int64_t(std::stoi(val));
  	// 	break;
  	// case T_ID_FLOAT:
  	// 	*((double *) dest) = double(std::stof(val));
  	// 	break;
    	
  	default:
  	{
  		// Type error: No conversion from string to target type
  		return nullptr;
  	}
  }
  // cout << "END" << endl;
  return dest;
}

template <typename T>
inline void* _copy_convert_from_numerical(void* dest, T&& val, CRE_Type* type){
	// cout << "BEFORE DEST: " << uint64_t(dest) << endl;
	// cout << "TYPE TID: " << uint64_t(type->t_id) << endl;
	switch(type->t_id) {
		case T_ID_BOOL:
			new (dest) bool(val);
			break;
    	case T_ID_INT:
    		new (dest) int64_t(val);
    		break;
    	case T_ID_FLOAT:
    		new (dest) double(val);
    		break;
    	case T_ID_STR:
    	{
    		std::string* s = (std::string*) new(dest) std::string(""); // Construct string in buffer
    		*s = std::to_string(val);
    		break;
    	}
    	default:
    	{
    		// Type error: No conversion from numerical to target type
    		return nullptr; 
    	}
  }
  return dest;
}

inline bool _check_pointer_is_of_type(void* ptr, CRE_Type* type){
	if(type->t_id == T_ID_PTR){
		return true;
	}else if(type->t_id >= T_ID_OBJ){
		CRE_Obj* obj = (CRE_Obj*) ptr;
		if(type->t_id > T_ID_OBJ && type->t_id != obj->get_t_id()){
			// Type error: t_id mismatch 
			return false; 
		}
		if(type->t_id == T_ID_FACT){
			// TODO: Check for inheritance 
		}
	}else{
		// Type error: Target type T_ID is not pointer type 
		return false; 
	}
	return true;
}

template <typename T>
void* copy_convert_arg(void* dest, T& val, CRE_Type* type){
	using Tb = std::remove_cv_t<T>;

	// Reference Types 
	if constexpr(std::is_pointer_v<Tb>){
		if(!_check_pointer_is_of_type(val, type)) return nullptr;
		new (dest) Tb(val);
		return dest;
	}else if constexpr(is_ref_v<Tb>){
		if(!_check_pointer_is_of_type(val, type)) return nullptr;

		new (dest) Tb::type*(val.get());
		// return _copy_convert_from_ptr(dest, Tb::type*(val.get()), type);
		return dest;

	// Untyped Item 
	}else if constexpr(std::is_same_v<Tb, Item>){
		const Item& val_item = val;
		switch(val.get_t_id()) {
		case T_ID_BOOL:
			return _copy_convert_from_numerical(dest, val_item.as<bool>(), type);
		case T_ID_INT:
			return _copy_convert_from_numerical(dest, val_item.as<int64_t>(), type);
		case T_ID_FLOAT:
			return _copy_convert_from_numerical(dest, val_item.as<double>(), type);
		case T_ID_STR:
			return _copy_convert_from_str(dest, val_item.as<std::string_view>(), type);
		default:
			if(val.is_ptr()){
				if(!_check_pointer_is_of_type(val_item.get_ptr(), type)) return nullptr;
				return val_item.get_ptr();	
			}else{
				return nullptr;
			}
		}

	// Numerical Types
	}else if constexpr(std::is_arithmetic_v<Tb>){
		return _copy_convert_from_numerical(dest, std::move(val), type);

	// String Types
	}else if constexpr(std::is_convertible_v<const T&, std::string_view>){
		return _copy_convert_from_str(dest, val, type);
	}else{
		// static_assert(0, );
		// std::stringstream ss;
		// ss << "Not Implemented for type: ";
		// ss << get_type_name<Tb>() << ".";
		// ss << get_type_name<T>() << ", ";
		// ss << ", " << std::is_arithmetic_v<Tb> << " " << std::is_arithmetic_v<T> << " " << std::is_arithmetic_v<int64_t> << endl;
		// std::is_arithmetic_v<Tb>
		// throw std::runtime_error(ss.str());

		// Type Error: Template Type Not Implemented
		return nullptr; 
	}
	// cout << "END_-" << endl;

}

struct StrBlock {
	std::string str;
	std::string_view view;

	StrBlock() : 
		str(""), view("") {};

	StrBlock(const char* _str) : 
		view(_str) {};

	StrBlock(const std::string& _str) : 
		str(_str), view(str) {};

	StrBlock(const std::string&& _str) : 
		str(_str), view(str) {};

	StrBlock(const std::string_view& view) : 
		view(view) {};
};


// SFINAE-based trait to check for operator<<
template <typename T, typename = void>
struct has_ostream_operator_impl : std::false_type {};

template <typename T>
struct has_ostream_operator_impl<T,
    std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>>
    : std::true_type {};

template <typename T>
constexpr bool has_ostream_operator_v = has_ostream_operator_impl<T>::value;

template <typename T>
constexpr auto _throw_type_name_helper() {
    if constexpr(std::is_same_v<T, Item>){
		return "cre::Item";
	}else{
		return typeid(T).name();
	}
}


void Func_dtor(const CRE_Obj* x);


struct Func;
typedef void (*StackCallFunc)(void*, void**);
typedef void (*StackCallFunc2)(void*, uint8_t*);
typedef Item (*PtrToItemFunc)(void*);
typedef void (*CallRecursiveFunc)(Func*, void*, void**);

struct Func : CRE_Obj{
	static constexpr uint16_t T_ID = T_ID_FUNC;

	CRE_Type* return_type;

	// The number of arguments taken by this cf.
	int64_t n_args;

	// The number of arguments taken by the original call() of this cf.
	int64_t n_root_args;

	// Keeps track of data pertaining to the base CREFunc that originated this 
    // CREFunc including a weak pointer, it's name and various ways of printing it.
	OriginData* origin_data;

	//
	uint8_t* bytecode = nullptr;
	uint8_t* bytecode_end = nullptr;

	size_t stack_size = 0;
	size_t outer_stack_size = 0;

	// The args to the root CREFunc 
	std::vector<ArgInfo> root_arg_infos = {};

	// The span of head_infos associated with each base Var.
	std::vector<HeadRange> head_ranges = {};

	// Keeps pointers to each head variable, their types, the CREFunc they live in 
    // among other things
  std::vector<HeadInfo> head_infos = {};

	// std::vector<InstrType> prereq_instrs;    

	// Data ptr of the return value    
	

	// The address for the root CREFunc's 'call_head' implementation.
    // Essentially the CREFunc's main call function.     
	// void* call_heads_addr;

	StackCallFunc stack_call_func;
	StackCallFunc2 stack_call_func2;
	PtrToItemFunc ptr_to_item_func;
	CallRecursiveFunc call_recursive_fc;

	// The address for the root CREFunc's 'call_self' implementation.
  // Calls call_heads on any values stored in 'h{i}' (i.e. 'head') slots.
  void* call_self_addr;

  // The address for the root CREFunc's 'resolve_heads_addr' implementation.
  //  Dereferences any objects in 'a{i}' (i.e. 'arg') slots and writes them
 	//  to each corresponding 'h{i}' (i.e. 'head') slot.
  void* resolve_heads_addr;

  // True if the func has beed initialized
  bool is_initialized=false;

  // True if this func is a ptr func
  bool is_ptr_func=false;

  // The composition depth
  uint16_t depth = 1;
  uint16_t return_stack_offset=0;


    

  bool has_any_derefs;

  bool is_composed;
  bool is_origin=false;


  Func(StackCallFunc _stack_call_func,
  		 StackCallFunc2 _stack_call_func2,
  		 PtrToItemFunc _ptr_to_item_func,
  		 CallRecursiveFunc _call_recursive_fc,
  	 size_t n_args,
  	 OriginData* _origin_data) :

  	n_args(n_args),
  	n_root_args(n_args),
  	origin_data(_origin_data),
  	stack_call_func(_stack_call_func),
  	stack_call_func2(_stack_call_func2),
  	ptr_to_item_func(_ptr_to_item_func),
  	call_recursive_fc(_call_recursive_fc)
  {
  	Item* data_ptr = std::bit_cast<Item*>(
		    std::bit_cast<uint64_t>(this) + sizeof(Func)
		);
	  for(size_t i=0; i < n_root_args; ++i){
			new (data_ptr + i) Item();
		}
  	// this->init_control_block(&Func_dtor, T_ID);
  }
  // Func(const Func&) = default;

  std::string to_string(uint8_t verbosity=DEFAULT_VERBOSITY);

  void set_arg(size_t i, const Item& val);

  template<class T>
  void set_arg(size_t i, const T& val){
  	set_arg(i, Item(val));
  }

  // TODO: Could this be done by overloading item instead? 
  // template<typename T>
  // void set_arg(size_t i, const ref<T> val){
	// 	set_arg(val.get());
	// }


  // void set_const_arg(size_t i, const Item& val);
  // void set_var_arg(size_t i, Var* val);
  // void set_func_arg(size_t i, Func* val);

  inline void set(uint32_t ind, const Item& val){
		Item* data_ptr = std::bit_cast<Item*>(
		    std::bit_cast<uint64_t>(this) + sizeof(Func)
		);

		// Emplacement new with Undef member to make valgrind happy
    
		// data_ptr[ind].release();
		data_ptr[ind] = val;
	}

	inline Item* get(uint32_t ind) const {
	    Item* data_ptr = std::bit_cast<Item*>(
	        std::bit_cast<uint64_t>(this) + sizeof(Func)
	    );
	    return &data_ptr[ind];
	}

	std::string bytecode_to_string();

	FuncRef copy_shallow();
	FuncRef copy_deep();
	
	void reinitialize();

// private:
	uint16_t calc_bytecode_length();

	void write_bytecode(
		uint16_t bytecode_len,
		uint16_t args_stack_length,
		const std::map<void*, size_t>& base_var_map,
		const std::vector<uint16_t>& arg_stack_offsets
	);

	void* call_stack(uint8_t* stack);
	void call_recursive2(void* ret_ptr, uint8_t* input_args);
	void _call_recursive2(void* ret_ptr, uint8_t* args_stack, uint8_t* input_args);

	void call_recursive(void* ret_ptr, void** head_val_ptrs);
	void _call_recursive(void* ret_ptr, void** head_val_ptrs);

	inline void throw_bad_n_args(int n_got){
		std::stringstream ss;
		ss << "Func Error: " << this << " with " << n_args << " arguments given " << n_got << " arguments." << endl;
		throw std::runtime_error(ss.str());
	}


	template<typename T>
	inline void throw_bad_arg(int i, T&& arg, CRE_Type* type){
		using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;

		std::stringstream ss;
		ss << "Bad cre::Func Argument. No Conversion of " ;
		
		if constexpr(has_ostream_operator_v<T>){
			ss << "\033[1;31m" <<  arg <<"\033[0m " ;
		}

		if constexpr(std::is_base_of_v<CRE_Obj, DecayT>){
			CRE_Type* inp_type = arg->get_type();			
			if(inp_type->t_id == T_ID_FACT){
				CRE_Type* f_type = arg->get_type();
				ss << "with type \033[1m" << f_type << "\033[0m ";
			}else{
				ss << "with type \033[1m" << inp_type << "\033[0m ";	
			}
		}else{ 
			ss << "with type " << "\033[1m" << _throw_type_name_helper<DecayT>() << "\033[0m ";
		}
		ss << "to type \033[1m" << type <<"\033[0m " <<
			  "for parameter " << i << " " <<  			
 			  "in cre::Func: " << this << endl;
		throw std::runtime_error(ss.str());
	}

	template <class ... Ts>
	FuncRef compose(Ts && ... args);

	template <class ... Ts>
	Item call(Ts&& ... args);

	template <class ... Ts>
	inline auto operator()(Ts&& ... args);
	

	// template <class... Ts>
	// Item operator()(Ts && ... inputs){
	// 	//TODO
	// }
};


// An alias of ref<Func> which has call operator() defined
struct FuncRef : ref<Func> {
	template <class ... Ts>
	inline auto operator()(Ts && ... inputs);
	// inline FuncRef operator()(Ts && ... inputs){
	// 	return get()->compose(*this, inputs...);
	// }
	
};


template <class ... Ts>
FuncRef Func::compose(Ts && ... args){
  FuncRef cf = this->copy_deep();//new Func(*this);

  if(sizeof...(args) != n_args){
  	throw_bad_n_args(sizeof...(args));
  }

  // cout << "COMPOSE: " << uint64_t(cf.get()) << endl;
  int i = 0;
  ([&]
    {
    		// cout << typeid(Ts).name() << endl;
    		// cout << "INPUT:" << inputs << endl;
    		cf->set_arg(i, args);	
        ++i;        
    } (), ...);
  cf->reinitialize();
  return cf;
}

template <typename T>
void* resolve_heads(void* dest, T&& input, const HeadInfo& hi){
	using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;

	void* head_val_ptr = nullptr;
	if(!hi.has_deref){
		head_val_ptr = copy_convert_arg((void*) dest, input, hi.base_type);
	}else{
	if constexpr(std::is_base_of_v<CRE_Obj, DecayT>){
		CRE_Obj* obj_ptr = nullptr;
		if constexpr(is_ref_v<T>){
			obj_ptr = (CRE_Obj*) input.get();	
		}else{
			obj_ptr = (CRE_Obj*) input;	
		}

		Var* var = hi.var_ptr;
		const Item* deref_result = var->apply_deref(obj_ptr);

		if(deref_result == nullptr or deref_result->get_t_id() == T_ID_UNDEF){
			std::stringstream ss;
			ss << "Var " << var << " de-reference failed from input " << input << "." << endl;
			throw std::runtime_error(ss.str());
		}
		head_val_ptr = copy_convert_arg((void*) dest, *deref_result, hi.head_type);	    		}
	}
	return head_val_ptr;
}

template <class ... Ts>
Item Func::call(Ts&& ... args){
	void** head_val_ptrs = (void**) alloca(sizeof(void**)*head_infos.size());
	uint8_t* stack = (uint8_t*) alloca(outer_stack_size);
	uint8_t* arg_head = stack;

	// cout << "STR SIZE: " << sizeof(std::string) << endl;
	// cout << "STACK SIZE: " << stack_size << endl;
	// cout << "OUTER STACK SIZE: " << outer_stack_size << endl;
	// cout << "RT B: " << return_type->byte_width << endl;
	// cout << "STACK: " << uint64_t(stack) << endl;
	// cout << "HEAD VAL PTRS SIZE: " << sizeof(void**)*head_infos.size() << endl;
	// cout << "HEAD VAL ADDR: " << uint64_t(head_val_ptrs) << endl;

	int64_t ret = 0;
	void* ret_ptr = (void*) (stack+outer_stack_size-return_type->byte_width);

	if(sizeof...(args) != n_args){
	  	throw_bad_n_args(sizeof...(args));
	}

	int i = 0;
  ([&]
    {	
		auto h_start = head_ranges[i].start;
		auto h_end = head_ranges[i].end;

    	// const HeadInfo& hi0 = head_infos[h_start];
    	// cout << "i=" << i << " h:" << head_ind << " V:" << args << " width=" << hi0.base_type->byte_width << " type=" << typeid(Ts).name() << endl;

    	for(size_t head_ind=h_start; head_ind<h_end; ++head_ind){
    		const HeadInfo& hi = head_infos[head_ind];
    		void* head_val_ptr = resolve_heads(arg_head, args, hi);

	    	if(head_val_ptr == nullptr){ [[unlikely]]
	    		throw_bad_arg(i, args, hi.base_type);	
	    	}
			
			head_val_ptrs[head_ind] = head_val_ptr;	
			arg_head += hi.base_type->byte_width;	    	
		}

        ++i;        
    } (), ...);

	// cout << "HEAD VAL PTRS 0: " << uint64_t(head_val_ptrs[0]) << endl;
 
  // call_recursive(ret_ptr, head_val_ptrs);
  call_recursive_fc(this, ret_ptr, head_val_ptrs);

  // TODO: This needs to be dynamic
  return ptr_to_item_func(ret_ptr);
  // return Item(*((int64_t*) ret_ptr));
}

template <class ... Ts>
inline auto Func::operator()(Ts&& ... args){
	// Check if any argument is Var or Func type
	if constexpr (has_var_or_func<Ts...>::value) {
		// If any argument is Var or Func, call compose
		return compose(std::forward<Ts>(args)...);
	} else {
		// Otherwise, call the function directly
		return call(std::forward<Ts>(args)...);
	}
}

template <class ... Ts>
inline auto FuncRef::operator()(Ts && ... args){
	return get()->operator()(std::forward<Ts>(args)...);
}





// ss << "Func Error: Incorrect Argument Type:" << typeid(T).name() <<
// 	 			" for argument i=" << i << " with type: " << type << 
// 	 			" in Func: " << func << endl;

//     		std::stringstream ss;
// 				ss << "Func Argument Error: No conversion of " << typeid(T).name() << ": " << val << " to " << type << 
// 					" for argument i=" << i << " with type: " << type << 
// 	 				" in Func: " << func << endl;
// 				endl;
//     		throw std::runtime_error(ss.str());



template<class T>
struct is_c_str
  : std::integral_constant<
      bool,
      std::is_same<char const *, typename std::decay<T>::type>::value ||
      std::is_same<char *, typename std::decay<T>::type>::value
> {};



/*

template <class ... Ts>
Item call2(Func* self, Ts&& ... args){
	// cout << "STACK SIZE: " << self->stack_size << endl;
	uint8_t* stack = (uint8_t*) alloca(self->stack_size);
	uint8_t* arg_head = stack;
	int64_t ret = 0;
	void* ret_ptr = (void*) (stack+self->return_stack_offset);

	// if(sizeof...(args) != self->n_args){
  // 	throw_bad_n_args(self, sizeof...(args));
  // }

	int i = 0;
  ([&]
    {
	    	const HeadInfo& hi0 = self->head_infos[self->head_ranges[i].start];

	    	// if(to_cre_type<Ts>() != hi0.base_type){
	    	// 	throw_bad_arg<Ts>(self, i, hi0.base_type);
	    	// }

	    	// cout << i << " V:" << args << " width=" << hi0.base_type->byte_width << " type=" << typeid(Ts).name() << endl;

	    	bool conv_error = copy_convert_arg((void*) arg_head, args, hi0.base_type);

	    	if(conv_error){ [[unlikely]]
	    		throw_bad_arg<Ts>(self, i, hi0.base_type);	
	    	}
	    	
	    	// Ts* _arg_ptr = (Ts*) arg_head;
	    	// _arg_ptr[0] = args;

	    	arg_head += hi0.base_type->byte_width;

	    	
	    	// cout << uint64_t(hi0.base_type) << "::" << self->head_ranges[i].start << endl;
	    	// cout << int64_t(arg_head)-int64_t(stack) << endl;

    		// cf->set_arg(i, args);	
        ++i;        
    } (), ...);

  // cout << "A STACK: " << uint64_t(stack) << " size=" << uint64_t(self->stack_size) << "; " << (uint64_t) *stack << ", " << (uint64_t) *(stack+8) << endl;

  // void* ret_ptr = self->call_stack(stack);
  // void* ret_ptr = alloca(sizeof(int64_t));
  // int64_t ret = 0;

  // self->call_recursive((void*) stack+self->return_stack_offset, stack);
  self->call_recursive2(ret_ptr, stack);

  // cout << "B STACK: " << uint64_t(stack) << " size=" << uint64_t(self->stack_size) << "; " << (uint64_t) *stack << ", " << (uint64_t) *(stack+8) << endl;

  
  // cout << "BLARG: " << int64_t(*((int64_t*) ret_ptr)) << endl;
  // cout << "BLARG: " << ret << endl;
  // cout << endl;
  // return Item(int64_t(*((int64_t*) ret_ptr)));

  // TODO: This needs to be dynamic
  return Item(ret);
  // return 
  // cf->reinitialize();
}



*/





// inline Func* _alloc_func(uint32_t n_args){
//   Func* ptr = (Func*) malloc(sizeof(Func) + (n_args) * sizeof(Item));
//   return ptr;
// }




std::ostream& operator<<(std::ostream& out, Func* func);
std::ostream& operator<<(std::ostream& out, ref<Func> func);


// ------------------------------------------------------------
// : SIZEOF_FUNC(n)

constexpr bool FUNC_ALIGN_IS_POW2 = (alignof(Func) & (alignof(Func) - 1)) == 0;
#define _SIZEOF_FUNC(n) (sizeof(Func)+(n)*sizeof(Member))

// Because Funcs are regularly allocated with buffers and have an atomic
//  for their refcount we need to make sure to pad Funcs so that
//  ((Func*) x) + SIZEOF_FUNC(x->size()) is always an aligned address so 
//  that we keep Funcs in contigous memory that wasn't allocated with 'new'
#if FUNC_ALIGN_IS_POW2 == true
  #define FUNC_ALIGN_PADDING(n_bytes) ((alignof(Func) - (n_bytes & (alignof(Func)-1))) & (alignof(Func)-1))
#else
  #define FUNC_ALIGN_PADDING(n_bytes) ((alignof(Func) - (n_bytes % (alignof(Func)))) % (alignof(Func)))
#endif

constexpr bool FUNC_NEED_ALIGN_PAD = (FUNC_ALIGN_PADDING(sizeof(Func)) | FUNC_ALIGN_PADDING(sizeof(Member))) != 0;

#if FUNC_NEED_ALIGN_PAD
  #define SIZEOF_FUNC(n) (_SIZEOF_FUNC(n) + FUNC_ALIGN_PADDING(_SIZEOF_FUNC(n)))
#else
  #define SIZEOF_FUNC(n) _SIZEOF_FUNC(n)
#endif





// Func* alloc_func(uint32_t length, AllocBuffer* buffer=nullptr);

// -----------------------------------------
// : stack_call<func>(...),  stack_call_func_ptr(&func, ...)

template <typename Arg, std::size_t I>
inline Arg read_arg(void** args) {
		using A  = std::remove_cvref_t<Arg>;
    return *( (A*) args[I]);
}


template <typename RT, typename... Args, std::size_t... I>
inline void stack_call_func_ptr_w_indices(RT (*func)(Args...), void* ret, void** args, std::index_sequence<I...>) {
    *((RT*) ret) = func(read_arg<Args, I>(args)...);
}


template <typename RT, typename... Args>
void stack_call_func_ptr(RT (*func)(Args...), void* ret, void** args) {
	stack_call_func_ptr_w_indices(
		func, ret, args, std::index_sequence_for<Args...>{}
	);
}


template<auto Func>
struct StackCallWrapper final
{
	template <typename RT, typename... Args, std::size_t... I>
	inline void variatic_call_w_indicies(RT (* )(Args...),
	 	void* ret, void** args,
	 	std::index_sequence<I...>) const {

			// Use placement new to initialize return value to make 
		  //  valgrind happy, since it should be uninitialized memory.
			if constexpr (std::is_same_v<RT, std::string>){
				// void* temp_addr = (void*) alloca(sizeof(std::string));
				// std::string* temp_str = new (temp_addr) RT(Func(read_arg<Args, I>(args)...));

				new (ret) StrBlock(RT(Func(read_arg<Args, I>(args)...)));

				// new (ret) std::string_view(
				// 	*(new (temp_addr) RT(Func(read_arg<Args, I>(args)...)))
				// ); 	
				cout << "ITER: " << (*(StrBlock*) ret).view 
					   << " @ " << uint64_t(ret)
					   << " L=" << ((StrBlock*) ret)->view.size()  << endl;
			}else{
				new (ret) RT(Func(read_arg<Args, I>(args)...)); 	
			}
	    // *((RT*) ret) = Func(read_arg<Args, I>(args)...);
	}	

	template <typename RT, typename... Args>
	inline void variatic_call(RT (* )(Args...),
	 	void* ret, void** args) const {

	    variatic_call_w_indicies(Func, ret, args, std::index_sequence_for<Args...>{});
	}	
    
  void operator()(void* ret, void** args) const
  {
      return variatic_call(Func, ret, args);
  }
};

template<auto F>
void stack_call(void* ret, void** args){
	StackCallWrapper<F>{}(ret, args);
}


template<auto F>
struct RecursiveCallWrapper final
{
	template <typename RT, typename... Args, std::size_t... I>
	inline void variatic_call_w_indicies(RT (* )(Args...),
	 	Func* self, void* ret, void** head_val_ptrs,
	 	std::index_sequence<I...>) const {
		
	// Stack initialize N argument pointers, and a tuple to write in 
	//  intermediate values, if any function returns or copies are necessary   
		void* arg_ptrs[sizeof...(Args)];
		std::tuple<std::remove_cvref_t<Args>...> temp;  // Remove refs from tuple
		// cout << "SIZEOF TEMP: "<< sizeof(std::tuple<std::remove_cvref_t<Args>...>) << endl;

	// Lambda apply the switch case over the known argument types for type specialization
	//   and to avoid overhead of looping. 
		([&] {	
	    	auto& arg_info = self->root_arg_infos[I];
			switch(arg_info.kind){
			case ARGINFO_FUNC:
			{
			// For a Func argument recurse into it.
				Func* func = self->get(I)->as_func();
				func->call_recursive_fc(func, &std::get<I>(temp), head_val_ptrs);
				arg_ptrs[I] = &std::get<I>(temp);
				break;
			}
			case ARGINFO_CONST:
			{
			// For constant arguments just point to the value of the relevant Item member 
				using DecayArg = std::remove_cvref_t<Args>;
				Item* const_val = self->get(I);
				if constexpr (std::is_same_v<DecayArg, StrBlock> ||
				 			  std::is_same_v<DecayArg, std::string_view> ||
				 			  std::is_same_v<DecayArg, std::string>){

				// For strings we need to copy into a StrBlock as a string_view.
					std::get<I>(temp) = StrBlock(const_val->as<std::string_view>());
					arg_ptrs[I] = &std::get<I>(temp);	
				}else{
					arg_ptrs[I] = (void*) &(const_val->val);
				}
				break;
			}
			case ARGINFO_VAR:
			{
			// For Var arguments we draw from the head_val_ptrs provided by the outer call,
			//   which points to values found by de-referencing variables (i.e. A.next.next.value). 
				arg_ptrs[I] = head_val_ptrs[arg_info.head_ind];
				break;
			}
			default:
				throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
			}
			// cout << "-Arg " << I << ":" << ((StrBlock*) arg_ptrs[I])->view << " @ " << uint64_t(arg_ptrs[I]) << endl;
        // ++i;        
    	} (), ...);

    	// ([&] {	
		// 	cout << " Arg " << I << ":" << ((StrBlock*) arg_ptrs[I])->view << " @ " << uint64_t(arg_ptrs[I]) << endl;
    	// } (), ...);

    	// for(size_t i=0; i < self->n_root_args; ++i){
		// 	cout << std::get<i>()
		// }

		// Call the Func at this level and insert the result into the return pointer. 
		if constexpr (std::is_same_v<RT, std::string>){
			// new (ret) StrBlock(std::move(F(read_arg<Args, I>(arg_ptrs)...)));
			new (ret) StrBlock(std::move(F(*(StrBlock*) arg_ptrs[I]...)));
			// cout << "Ret " << ":" << ((StrBlock*) ret)->view << " @ " << uint64_t(ret) << endl;
		}else{
			// new (ret) RT(F(read_arg<Args, I>(arg_ptrs)...)); 	
			new (ret) RT(F( *((Args*) arg_ptrs[I]) ...)); 	
		}

		// ([&] {	
		// 	cout << "+Arg " << I << ":" << ((StrBlock*) arg_ptrs[I])->view << " @ " << uint64_t(arg_ptrs[I]) << endl;
    	// } (), ...);
	}	

	template <typename RT, typename... Args>
	inline void variatic_call(RT (* )(Args...),
	 	Func* self, void* ret, void** head_val_ptrs) const {

	    variatic_call_w_indicies(F, self, ret, head_val_ptrs, std::index_sequence_for<Args...>{});
	}	
    
  void operator()(Func* self, void* ret, void** head_val_ptrs) const
  {
      return variatic_call(F, self, ret, head_val_ptrs);
  }
};


template<auto F>
void call_recursive_fc(Func* self, void* ret_ptr, void** head_val_ptrs){
	RecursiveCallWrapper<F>{}(self, ret_ptr, head_val_ptrs);
}




// template <typename RT, typename... Args, std::size_t... O>
// inline void stack_call_func_ptr_w_indices2(RT (*func)(Args...), void* ret, uint8_t* args, std::index_sequence<O...>) {
//     *((RT*) ret) = func(read_arg<Args, O>(args)...);
// }


// template <typename RT, typename... Args>
// void stack_call_func_ptr2(RT (*func)(Args...), void* ret, uint8_t* args) {
// 	stack_call_func_ptr_w_indices(
// 		func, ret, args, std::index_sequence_for<Args...>{}
// 	);
// }


#include <cstddef>
#include <array>
// #include <vector> // If dynamic array is acceptable

// template <typename... Ts>
// struct RegType{

// 	static constexpr 
// 	if constexpr(std::is_same_v<Ts, std::string> || std::is_same_v<Ts, std::string_view>){
// 		using ArgTy = StrBlock;
// 	}else{
// 		using ArgTy = Ts;
// 	}
// };


template <typename... Ts>
struct AlignedLayout {
    static constexpr std::array<std::size_t, sizeof...(Ts)> calculate_offsets() {
        std::array<std::size_t, sizeof...(Ts)> offsets_arr;
        std::size_t current_offset = 0;
        std::size_t index = 0;

        // Fold expression to calculate offsets
        ((
            current_offset = (current_offset + alignof(Ts) - 1) / alignof(Ts) * alignof(Ts),
            offsets_arr[index++] = current_offset,
            current_offset += sizeof(Ts)
        ), ...);

        return offsets_arr;
    }

    static constexpr std::array<std::size_t, sizeof...(Ts)> offsets = calculate_offsets();

    static constexpr std::size_t total_size = []{
        std::size_t current_offset = 0;
        ((
            current_offset = (current_offset + alignof(Ts) - 1) / alignof(Ts) * alignof(Ts),
            current_offset += sizeof(Ts)
        ), ...);
        return current_offset;
    }();
    
    static constexpr std::size_t max_align = []{
        std::size_t max_a = 1;
        ((max_a = std::max(max_a, alignof(Ts))), ...);
        return max_a;
    }();
};




template<auto F>
struct StackCallWrapper2 final
{
	template <typename RT, typename... Args, size_t... I>
	constexpr void variatic_call_w_offsets(RT (* )(Args...),
	 	void* ret, uint8_t* args,
	 	std::index_sequence<I...>,
	 	std::array<size_t, sizeof...(Args)> offsets
	 	) const {

		// ([&] {
			// using DecayedArg = std::remove_cvref_t<Args>;

		// 	// cout << *( (Args*) &args[offsets[I]]) << endl;
		// 	cout << "O:" << offsets[I] << 
		// 	     " ^^:" << (uint64_t) &args[offsets[I]] << 
		// 		   " V:" << *( (Args*) &(args[offsets[I]])) << 
		// 		   " BW:" << sizeof(Args) << " " << typeid(Args).name() <<
		// 	endl;
		// }(), ...);


	  *((RT*) ret) = F( *( (std::remove_cvref_t<Args>*) &args[offsets[I]]) ...);
	}	

	template <typename RT, typename... Args>
	constexpr void variatic_call(RT (* )(Args...),
	 	void* ret, uint8_t* args) const {

	    variatic_call_w_offsets(F, ret, args, 
	    		std::index_sequence_for<Args...>{},
	    		AlignedLayout<Args...>::offsets
	    );
	}	
    
  void operator()(void* ret, uint8_t* args) const
  {
      return variatic_call(F, ret, args);
  }
};

template<auto F>
void stack_call2(void* ret, uint8_t* args){
	StackCallWrapper2<F>{}(ret, args);
}


// ------------------------------------------------------


// -----------------------------------------
// : stack_call<func>(...),  stack_call_func_ptr(&func, ...)

template <typename Arg, std::size_t I>
inline Arg read_arg_generic(Item* args) {
    return args[I].as<Arg>();
}

// template <typename RT, typename... Args, std::size_t... I>
// inline Item stack_call_func_ptr_w_indices_v(RT (*func)(Args...), Item* args, std::index_sequence<I...>) {
//     return Item(func(read_arg_v<Args, I>(args)...));
// }


// template <typename RT, typename... Args>
// Item stack_call_func_ptr_v(RT (*func)(Args...), Item* args) {
// 	return stack_call_func_ptr_w_indices_v(
// 		func, args, std::index_sequence_for<Args...>{}
// 	);
// }


template<auto F>
struct StackCallFuncGeneric final
{
	template <typename RT, typename... Args, std::size_t... I>
	inline Item variatic_call_w_indicies(RT (* )(Args...),
	 	Item* args, std::index_sequence<I...>) const {
	    return Item(F(read_arg_generic<Args, I>(args)...));
	}	

	template <typename RT, typename... Args>
	inline Item variatic_call(RT (* )(Args...), Item* args) const {
	    return variatic_call_w_indicies(F, args, std::index_sequence_for<Args...>{});
	}	
    
  Item operator()(Item* args) const {
      return variatic_call(F, args);
  }
};

template<auto F>
Item stack_call_generic(Item* args){
	return StackCallFuncGeneric<F>{}(args);
}

// ------------------------------------------------------



ref<Func> new_func(
	StackCallFunc stack_call_func,
	StackCallFunc2 stack_call_func2,
	PtrToItemFunc ptr_to_item_func,
	CallRecursiveFunc call_recursive_fc,
	size_t n_args, OriginData* origin_data, AllocBuffer* buffer=nullptr
);

FuncRef define_func(
		const std::string_view& name, 
		StackCallFunc cfunc_ptr,
		StackCallFunc2 cfunc_ptr2,
		PtrToItemFunc ptr_to_item_func,
		CallRecursiveFunc call_recursive_fc,
		size_t stack_size,
		const std::vector<uint16_t>& offsets,
		CRE_Type* ret_type,
		const std::vector<CRE_Type*>& arg_types,
		const std::string_view& expr_template = "",
		const std::string_view& shorthand_template = "");



// -- Helper struct for extracting RT and ARGS --
template <typename T>
struct FuncToCRETypes;

template <typename RT, typename... Args>
struct FuncToCRETypes<RT(*)(Args...)> {

		static std::tuple<CRE_Type*, std::vector<CRE_Type*>, size_t, std::vector<uint16_t>>
      get(){
				CRE_Type* ret_type = to_cre_type<RT>();
				std::vector<CRE_Type*> arg_types = {to_cre_type<Args>()...};		

				using arg_layout = AlignedLayout<Args..., RT>;

				size_t stack_size;
				if constexpr (std::is_same_v<RT, std::string>){
					stack_size = arg_layout::total_size + sizeof(std::string_view);
				}else{
					stack_size = arg_layout::total_size + sizeof(RT);	
				}
				
				std::vector<uint16_t> offsets = {};
				offsets.reserve(sizeof...(Args)+1);
				// for(int i=0; i < sizeof...(Args); i++){
				for(auto offset : arg_layout::offsets){
					offsets.push_back(offset);
				}
				// std::vector(arg_layout::offsets);

				return {ret_type, arg_types, stack_size, offsets};
		}
};


template <typename T>
inline Item _ptr_to_item(void* ret){
	if constexpr(std::is_same_v<T, std::string>){
		return Item( std::string(((StrBlock*) ret)->view) );
	}else{
		return Item(*((T*) ret));		
	}
}
 
template <typename T>
struct PtrToItem;

template <typename RT, typename... Args>
struct PtrToItem<RT(*)(Args...)> {
    static Item as_item(void* ret){
		return _ptr_to_item<RT>(ret);
	}
};

template<typename T>
Item ptr_to_item(void* ret){
	if constexpr(std::is_function_v<T>){
		return PtrToItem<T>::as_item(ret);	
	}else{
		return _ptr_to_item<T>(ret);
		// return Item(*((T*) ret));	
	}
} 


template <auto F>
FuncRef define_func(
		const std::string_view& name, 
		const std::string_view& expr_template = "",
		const std::string_view& shorthand_template = ""){

  auto stack_call_lambda = [](void* ret, void** args) {
      stack_call<F>(ret, args);
  };

  auto stack_call_lambda2 = [](void* ret, uint8_t* args) {
      stack_call2<F>(ret, args);
  };

  auto ptr_to_item_func_lambda = [](void* ret) {
      return PtrToItem<decltype(F)>::as_item(ret);
  };

  auto call_recursive_fc_lambda = [](Func* self, void* ret_ptr, void** head_val_ptrs) {
      // call_recursive_fc<decltype(F)>(self, ret, args);
      RecursiveCallWrapper<F>{}(self, ret_ptr, head_val_ptrs);
  };


	// void (*cfunc_ptr)(void* ret, void** args) = stack_call_lambda;
	StackCallFunc stack_call_func = stack_call_lambda;
	StackCallFunc2 stack_call_func2 = stack_call_lambda2;
	PtrToItemFunc ptr_to_item_func = ptr_to_item_func_lambda;
	CallRecursiveFunc call_recursive_func = call_recursive_fc_lambda;
	auto [ret_type, arg_types, stack_size, offsets] =
	 			FuncToCRETypes<decltype(F)>::get();

	// cout << offsets << endl;



	return define_func(name, 
		stack_call_func, stack_call_func2, ptr_to_item_func, call_recursive_func,
		 stack_size, offsets, ret_type, arg_types, expr_template, shorthand_template);
}


} // NAMESPACE_END(cre)

/*

TODO: 

Need to be aligned with way things worked in Python version of this 
with respect to having Var de-references resolved in into heads
before applying.

Should: 
1) Resolve all heads into an head_arg_stack
2) Every root_arg with T_ID_VAR should know its offset on the head_arg_stack
	- The head_arg_stack will consist only of copies of things that live
		a. Externally, they were passed as args.
		b. In another object, they are from an Item in a Fact, List, or Dict
	- Question is can we have these copies while having them live outside 
	  of normal RAI life cycle? We want to copy them temporarily as structs 
	  without copying their resources, or calling their destructors when we're
	  done with them. 
	- Perhaps we can just mem-copy them?
	- For Item types and de-referenced Items we need to convert from Item

	Alternatively we could just use pointers, which does seem to be relatively 
	fast but creates a similar problem where we need to temporarily copy any Item
	reference into it's appropriate base type. This is only necessary for de-references
	and Item args.
	



3) Run call_recursive, this a root_arg_stack is allocated for each call 
4) Need to cleanup certain situations


There is a sub-problem related to returning an untyped Item from
the typed execution of Funcs.   

*/
