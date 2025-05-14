#pragma once

#include <iostream>
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
	Item* ptr;
	uint16_t kind;
	// uint16_t t_id;
	// uint16_t byte_width;

	// The offset that the arg is written to in the call-stack
	uint16_t offset;
	bool has_deref;
	


	ArgInfo(CRE_Type* _type, Item* _ptr,
	 		uint16_t _kind, uint16_t _offset, bool _has_deref) :
		type(_type), ptr(_ptr), kind(_kind), 
		offset(_offset), has_deref(_has_deref)
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


void Func_dtor(const CRE_Obj* x);

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

	// The args to the root CREFunc 
	std::vector<ArgInfo> root_arg_infos = {};

	// The span of head_infos associated with each base Var.
	std::vector<HeadRange> head_ranges = {};

	// Keeps pointers to each head variable, their types, the CREFunc they live in 
    // among other things
  std::vector<HeadInfo> head_infos = {};

	// std::vector<InstrType> prereq_instrs;    

	// Data ptr of the return value    
	void* return_data_ptr;

	// The address for the root CREFunc's 'call_head' implementation.
    // Essentially the CREFunc's main call function.     
	void* call_heads_addr;

	// The address for the root CREFunc's 'call_self' implementation.
    // Calls call_heads on any values stored in 'h{i}' (i.e. 'head') slots.
    void* call_self_addr;

    // The address for the root CREFunc's 'resolve_heads_addr' implementation.
    //  Dereferences any objects in 'a{i}' (i.e. 'arg') slots and writes them
   	//  do each corresponding 'h{i}' (i.e. 'head') slot.
    void* resolve_heads_addr;

    // True if the func has beed initialized
    bool is_initialized;

    // True if this func is a ptr func
    bool is_ptr_func;

    // The composition depth
    uint16_t depth = 1;

    

    bool has_any_derefs;

    bool is_composed;


    Func(void* _cfunc_ptr,
    	 size_t n_args,
    	 OriginData* _origin_data) :

    	n_args(n_args),
    	n_root_args(n_args),
    	origin_data(_origin_data),
    	call_heads_addr(_cfunc_ptr)
    {
    	this->init_control_block(&Func_dtor);
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

    inline void set(uint32_t a_id, const Item& val){
		Item* data_ptr = std::bit_cast<Item*>(
		    std::bit_cast<uint64_t>(this) + sizeof(Func)
		);
		data_ptr[a_id] = val;
	}

	inline Item* get(uint32_t a_id) const {
	    Item* data_ptr = std::bit_cast<Item*>(
	        std::bit_cast<uint64_t>(this) + sizeof(Func)
	    );
	    return &data_ptr[a_id];
	}

	std::string bytecode_to_string();

	FuncRef copy_shallow();
	FuncRef copy_deep();
	size_t calc_byte_code_size();
	void reinitialize();
};



struct FuncRef : ref<Func> {
	template <class ... Ts>
	inline FuncRef operator()(Ts && ... inputs){
		return compose(*this, inputs...);
	}
};


template <class ... Ts>
FuncRef compose(Func* self, Ts && ... inputs){
  FuncRef cf = self->copy_deep();//new Func(*this);

  // cout << "COMPOSE: " << uint64_t(cf.get()) << endl;
  int i = 0;
  ([&]
    {
    		// cout << typeid(Ts).name() << endl;
    		// cout << "INPUT:" << inputs << endl;
    		cf->set_arg(i, inputs);	
        ++i;        
    } (), ...);
  cf->reinitialize();
  return cf;
}



// inline Func* _alloc_func(uint32_t n_args){
//   Func* ptr = (Func*) malloc(sizeof(Func) + (n_args) * sizeof(Item));
//   return ptr;
// }


ref<Func> new_func(size_t n_args, void* cfunc_ptr, OriginData* origin_data);

FuncRef define_func(
		const std::string_view& name, 
		void* cfunc_ptr,
		CRE_Type* ret_type,
		const std::vector<CRE_Type*>& arg_types,
		const std::string_view& expr_template = "",
		const std::string_view& shorthand_template = "");

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
  #define ALIGN_PADDING(n_bytes) ((alignof(Func) - (n_bytes & (alignof(Func)-1))) & (alignof(Func)-1))
#else
  #define ALIGN_PADDING(n_bytes) ((alignof(Func) - (n_bytes % (alignof(Func)))) % (alignof(Func)))
#endif

constexpr bool FUNC_NEED_ALIGN_PAD = (ALIGN_PADDING(sizeof(Func)) | ALIGN_PADDING(sizeof(Member))) != 0;

#if FUNC_NEED_ALIGN_PAD
  #define SIZEOF_FUNC(n) (_SIZEOF_FUNC(n) + ALIGN_PADDING(_SIZEOF_FUNC(n)))
#else
  #define SIZEOF_FUNC(n) _SIZEOF_FUNC(n)
#endif

inline Func* _alloc_func(uint32_t _length){
  Func* ptr = (Func*) malloc(SIZEOF_FUNC(_length));
  return ptr;
}


// -----------------------------------------
// : stack_call<func>(...),  stack_call_func_ptr(&func, ...)

template <typename Arg, std::size_t I>
inline Arg read_arg(uint8_t* data_start, uint16_t* arg_offsets) {
    return *( (Arg*) (data_start+arg_offsets[I]));
}


template <typename RT, typename... Args, std::size_t... I>
inline void stack_call_func_ptr_w_indices(RT (*func)(Args...), uint8_t* ret, uint16_t* arg_offsets, std::index_sequence<I...>) {
    *((RT*) ret) = func(read_arg<Args, I>(ret, arg_offsets)...);
}


template <typename RT, typename... Args>
void stack_call_func_ptr(RT (*func)(Args...), uint8_t* ret, uint16_t* arg_offsets) {
	stack_call_func_ptr_w_indices(
		func, ret, arg_offsets, std::index_sequence_for<Args...>{}
	);
}


template<auto Func>
struct StackCallFunc final
{
	template <typename RT, typename... Args, std::size_t... I>
	inline void variatic_call_w_indicies(RT (* )(Args...),
	 	uint8_t* ret, uint16_t* arg_offsets,
	 	std::index_sequence<I...>) const {

	    *((RT*) ret) = Func(read_arg<Args, I>(ret, arg_offsets)...);
	}	

	template <typename RT, typename... Args>
	inline void variatic_call(RT (* )(Args...),
	 	uint8_t* ret, uint16_t* arg_offsets) const {

	    variatic_call_w_indicies(Func, ret, arg_offsets, std::index_sequence_for<Args...>{});
	}	
    
  void operator()(uint8_t* ret, uint16_t* arg_offsets) const
  {
      return variatic_call(Func, ret, arg_offsets);
  }
};

template<auto Func>
void stack_call(uint8_t* ret, uint16_t* arg_offsets){
	StackCallFunc<Func>{}(ret, arg_offsets);
}

// ------------------------------------------------------

} // NAMESPACE_END(cre)
