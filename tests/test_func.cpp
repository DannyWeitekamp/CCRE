#include "../include/helpers.h"
#include "../include/fact.h"
#include "../include/func.h"
#include "../include/item.h"
#include "../include/var.h"
#include "test_macros.h"
#include <random>

#include <fmt/format.h>
#include <fmt/ranges.h>

using std::cout;
using std::endl;
using namespace cre;
// template<typename func>
// struct test_func {
// 	Item& operator()(Item&, ...){

// 	}
// };

int64_t add(int64_t a, int64_t b){
	return a+b;
}

typedef int64_t (*two_int_func_t)(int64_t a, int64_t b);
two_int_func_t add_ptr = &add;

void test_define(){
	FuncRef add_f = define_func("add", (void*) &add, cre_int, {cre_int, cre_int});
	cout << "add_f: " << add_f << endl;
	cout << "add_f: " << add_f << endl;


	ref<Var> A = new_var("A", cre_int);
	ref<Var> B = new_var("B", cre_int);
	ref<Var> C = new_var("C", cre_int);
	ref<Var> D = new_var("D", cre_int);

	// Set Const
	FuncRef f_1_A = add_f(1, A);
	
	cout << "add_f: " << add_f << endl;
	cout << "f_1_A: " << f_1_A << endl;

	// cout << f_1_A->bytecode_to_string() << endl;

	cout << "----------------" << endl;
	// Compose Func
	FuncRef f_1_A_B = add_f(f_1_A, B);
	cout << "f_1_A_B:" << f_1_A_B << endl;
	// cout << f_1_A_B->bytecode_to_string() << endl;

	cout << "----------------" << endl;
	// FuncRef big = f_1_A_B(f_1_A, 9);
	// cout << "f_1_A_B copy:" << f_1_A_B->copy_deep() << endl;

	// Deep Compose Func
	FuncRef big = f_1_A_B(9, f_1_A);
	cout << "big:" << big << endl;


	// Deep Compose Repeat Vars
	cout << "----------------" << endl;
	FuncRef dub = f_1_A_B(A, add_f(B, A));
	cout << "dub:" << dub << endl;
	cout << "dub:" << dub->n_args << endl;

	// Deep Compose Repeat Vars; insert const
	FuncRef dub_const = dub(1, A);
	cout << "dub_const:" << dub_const << endl;	


	// Deep Compose Repeat Vars; insert funcs
	cout << "<----------------" << endl;
	FuncRef dub_func = dub(add_f(A,B), add_f(C,D));
	cout << "dub_func:" << dub_func << endl;	
}

void test_compose_derefs(){
	FuncRef add_f = define_func("add_flt", (void*) &add, cre_float, {cre_float, cre_float});

	FactType* Person = define_fact("Person", 
	    {{"id", cre_str, {{"unique_id", true}}},
	     {"money", cre_float},
	     {"mom", new DefferedType("Person")},
	     {"dad", new DefferedType("Person")},
	     {"best_bud", new DefferedType("Person")}
	 	}
	);

	ref<Fact> olpops = make_fact(Person, "Ol'Pops", 3.50);
	ref<Fact> pops = make_fact(Person, "Pops", 100.0, nullptr, olpops, nullptr);
	ref<Fact> ma = make_fact(Person, "Ma", 0.0, pops);
	ref<Fact> ricky = make_fact(Person, "Ricky", 7.11);
	ref<Fact> thedude = make_fact(Person, "TheDude", 23131.73, ricky, pops, ma);


	cout << "Ol'Pops: " << olpops << endl;
	cout << "Pops: " << pops << endl;
	cout << "Ma: " << ma << endl;
	cout << "Ricky: " << ricky << endl;
	cout << "TheDude: " << thedude << endl;

	ref<Var> A = new_var("A", Person);
	ref<Var> B = new_var("B", Person);

	ref<Var> Ad = A->extend_attr("dad");
	ref<Var> Bd = B->extend_attr("dad");

	FuncRef dadd_m = add_f(Ad->extend_attr("money"), Bd->extend_attr("money"));
	cout << dadd_m;

	cout << "--------------" << endl;
	FuncRef bdadd_m = dadd_m(A->extend_attr("best_bud"), B->extend_attr("best_bud"));
	cout << bdadd_m;

	

}



Item add_items(Item* args){
	int a = args[0].as_int();
	int b = args[1].as_int();
	return Item(a + b);
}

typedef Item (*item_func_t)(Item* args);

item_func_t add_items_ptr = &add_items;

int64_t add_alloca(int64_t a, int64_t b){
	Item* stack = (Item*) alloca(sizeof(Item)*3);
	auto func = (*add_items_ptr);
	stack[1] = Item(a);
	stack[2] = Item(b);
	stack[0] = func(stack+1);
	return stack[0].as_int();
}

int64_t add_item_no_stack(int64_t a, int64_t b){
	// Item* stack = (Item*) alloca(sizeof(Item)*3);
	auto func = (*add_items_ptr);
	auto ia = Item(a);
	auto ib = Item(b);
	return (*add_ptr)(ia.as_int(), ib.as_int());
}

void add_ptrs(uint8_t* ret, uint16_t* arg_offsets){
	int64_t a = *( (int64_t*) (ret+arg_offsets[0]));
	int64_t b = *( (int64_t*) (ret+arg_offsets[1]));
	// cout << "o0:" << arg_offsets[0] << endl;
	// cout << "o1:" << arg_offsets[1] << endl;
	// cout << "a:" << a << endl;
	// cout << "b:" << a << endl;
	*((int64_t*) ret) = a+b;
	// return a+b;
}

typedef void (*abs_func_t)(uint8_t* ret, uint16_t* arg_offsets);
abs_func_t add_ptrs_ptr = &add_ptrs;



// template <typename F>
// ref<Fact> stack_call_wrapper(uint8_t* ret, uint16_t* arg_offsets){
//   // std::cout << std::endl;
//   Item items[sizeof...(Ts)];
//   // std::vector<Item> items = {};
//   // items.reserve();
//   int i = 0;
//   ([&]
//     {
//         // Do things in your "loop" lambda
//         // Item item = to_item(inputs);
//         // items.push_back(item);
//         items[i] = Item(inputs);
//         ++i;
        
//     } (), ...);
//   return new_fact(type, items, i);
// }

template <typename T>
void printType() {
    std::cout << typeid(T).name() << std::endl;
}



// template <typename RT, typename... Args, std::size_t... I>
// void variatic_call_abstract_func(RT (*func)(Args...), uint8_t* ret, uint16_t* arg_offsets, std::index_sequence<I...>) {
//     *((RT*) ret) = func(read_arg<Args, I>(ret, arg_offsets)...);
// }


// template <auto F>
// void bloop(uint8_t* ret, uint16_t* arg_offsets) {
	// printType<F>();
	// using Args = typename std::tuple_element<0, F>::type;
    // using RT = typename decltype(F)::result_type;
    // printType<RT>();

	// variatic_call_abstract_func(
	// 	F, ret, arg_offsets, std::index_sequence_for<Args...>{}
	// );

// }

int64_t run_add(){
	// uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	// int64_t* int_stack = (int64_t*) stack;
	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		t += (t>>8) + i;
	} 
	return t;
}


int64_t run_stack_call_func_ptr(){
	uint16_t arg_offsets[2] = {8,16};
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		int_stack[0] = 0;
		int_stack[1] = t>>8;
		int_stack[2] = i;
		stack_call_func_ptr(add_ptr, stack, arg_offsets);
		t += int_stack[0];
	} 
	return t;
}

int64_t run_stack_call(){
	uint16_t arg_offsets[2] = {8,16};
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		int_stack[0] = 0;
		int_stack[1] = t>>8;
		int_stack[2] = i;
		stack_call<add>(stack, arg_offsets);
		t += int_stack[0];
	} 
	return t;
}




int main(){
	// test_define();
	test_compose_derefs();

	return 0;

	auto func = (*add_items_ptr);

	time_it_n("inline", add(1,2), 100000);
	time_it_n("opaque_ptr", (*add_ptr)(1,2) ,100000);
	time_it_n("item_alloca", add_alloca(1,2) ,100000);
	time_it_n("add_item_no_stack", add_item_no_stack(1,2) ,100000);

	uint16_t arg_offsets[2] = {8,16};
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	int_stack[1] = 1;
	int_stack[2] = 2;

	add_ptrs(stack, arg_offsets);
	time_it_n("add_abs_extern_stack", add_ptrs(stack, arg_offsets) ,100000);
	cout << int_stack[0] << "," << int_stack[1] << ", " << int_stack[2] << endl;

	int_stack[0] = 0;

	cout << int_stack[0] << "," << int_stack[1] << ", " << int_stack[2] << endl;


	two_int_func_t add_ptr = two_int_func_t(uint64_t(&add));

	int64_t out = 0;

	time_it_n("run_add", out=run_add(); ,100000);

	cout << out << endl;

	time_it_n("run_stack_call_func_ptr", out=run_stack_call_func_ptr(); ,100000);
	cout << out << endl;
	
	// call_abstract_func(add, stack, arg_offsets);

	// cout << int_stack[0] << "," << int_stack[1] << ", " << int_stack[2] << endl;

	// int_stack[0] = 0;

	// cout << int_stack[0] << "," << int_stack[1] << ", " << int_stack[2] << endl;

	time_it_n("stack_call<add>", out=run_stack_call(); ,100000);
	cout << out << endl;


	void (*scf_add_ptr)(uint8_t*, uint16_t*) = &stack_call<add>;
	cout << uint64_t(scf_add_ptr) << endl;
	// FuncWrapper<add>{}(stack, arg_offsets);

	// cout << int_stack[0] << "," << int_stack[1] << ", " << int_stack[2] << endl;


	// bloop<add>(stack, arg_offsets);

	return 0;
}
