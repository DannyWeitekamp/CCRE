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

bool bin_and(bool a, bool b){
	return a & b;
}

int64_t add(int64_t a, int64_t b){
	return a+b;
}

double add_flt(double a, double b){
	return a+b;
}

// std::string concat(const std::string_view& a, const std::string_view& b){
std::string concat(const StrBlock& _a, const StrBlock& _b){
	std::string_view a = _a.view;
	std::string_view b = _b.view;
	std::stringstream ss;
	ss << a << b;
	// cout << "CONCAT A=" << a << " B="<< b << " A+B=" << ss.str() << endl;
	return ss.str();
	// return a+b;
}

// std::string paren(const std::string_view& x){
std::string paren(const StrBlock& _x){
	std::string_view x = _x.view;
	std::stringstream ss;
	ss << "(" << x << ")";
	return ss.str();
	// return a+b;
}

double add_money(Fact* a, Fact* b){
	return a->get("money").as<double>()+b->get("money").as<double>();
}


const auto add_stack = stack_call<add>;
// void add_stack(void* ret, void** args){
// 	stack_call<add>(ret, args);	
// }


typedef int64_t (*two_int_func_t)(int64_t a, int64_t b);
two_int_func_t add_ptr = &add;

void test_basic_int(){
	ref<Var> A = new_var("A", cre_int);
	ref<Var> B = new_var("B", cre_int);
	ref<Var> C = new_var("C", cre_int);
	ref<Var> D = new_var("D", cre_int);

	{ // Start New Frame 

	cout << "-----add_f------" << endl;
	// FuncRef add_f = define_func("add", add);
	FuncRef add_f = define_func<add>("add");
	// IS_TRUE(add_f(1,2).as<int64_t>() == 3);

	
	// FuncRef add_f = define_func("add", (void*) &add_stack, cre_int, {cre_int, cre_int});
	// cout << "add_f: " << add_f << endl;
	// cout << "bc_len=" << add_f->calc_bytecode_length() << endl;
	// cout << add_f->bytecode_to_string() << endl;

	// Set Const
	cout << "-----f_1_A------" << endl;
	FuncRef f_1_A = add_f(1, A);
	cout << "f_1_A: " << f_1_A << endl;
	// cout << "bc_len=" << f_1_A->calc_bytecode_length() << endl;
	// cout << f_1_A->bytecode_to_string() << endl;
	
	// cout << "CALL:" << f_1_A(int64_t(10)).as<int64_t>() << endl;
	IS_TRUE(f_1_A(2).as<int64_t>() == 3);

	// return;

	// cout << "A=" << A.get_refcount() << endl;
	// cout << "B=" << B.get_refcount() << endl;

	cout << "-----f_1_A_B------" << endl;
	// Compose Func
	FuncRef f_1_A_B = add_f(f_1_A, B);
	cout << "f_1_A_B:" << f_1_A_B << endl;
	// cout << "bc_len=" << f_1_A_B->calc_bytecode_length() << endl;
	// cout << f_1_A_B->bytecode_to_string() << endl;

	// cout << "CALL:" << f_1_A_B(3,7).as<int64_t>() << endl;
	IS_TRUE(f_1_A_B(3,7).as<int64_t>() == 11);



	// return;
	// cout << "A=" << A.get_refcount() << endl;
	// cout << "B=" << B.get_refcount() << endl;

	// cout << "-----f_1_A_B-copy----" << endl;
	// FuncRef cpy = f_1_A_B->copy_deep();//f_1_A_B(f_1_A, 9);
	// cout << "f_1_A_B copy:" << cpy << endl;


	cout << "-----big--------" << endl;
	// Deep Compose Func
	FuncRef big = f_1_A_B(9, f_1_A);
	cout << "big:" << big << endl;
	// cout << "bc_len=" << big->calc_bytecode_length() << endl;
	// cout << big->bytecode_to_string() << endl;
	// cout << "CALL:" << big(7).as<int64_t>() << endl;
	IS_TRUE(big(7).as<int64_t>() == 18);
	// return;

	// cout << "A=" << A.get_refcount() << endl;
	// cout << "B=" << B.get_refcount() << endl;



	// Deep Compose Repeat Vars
	cout << "-----dub--------" << endl;
	FuncRef dub = f_1_A_B(A, add_f(B, A));
	cout << "dub:" << dub << endl;


	// cout << "bc_len=" << dub->calc_bytecode_length() << endl;
	// cout << dub->bytecode_to_string() << endl;
	IS_TRUE(dub(2,7).as<int64_t>() == 12);
	// return;

	// Deep Compose Repeat Vars; insert const
	cout << "-----dub const--------" << endl;
	FuncRef dub_const = dub(1, A);
	cout << "dub_const:" << dub_const << endl;
	// cout << "bc_len=" << dub_const->calc_bytecode_length() << endl;
	// cout << dub_const->bytecode_to_string() << endl;	
	IS_TRUE(dub_const(7).as<int64_t>() == 18);


	// Deep Compose Repeat Vars; insert funcs
	cout << "-----dub func--------" << endl;
	FuncRef dub_func = dub(add_f(A,B), add_f(C,D));
	cout << "dub_func:" << dub_func << endl;
	// cout << "bc_len=" << dub_func->calc_bytecode_length() << endl;
	// cout << dub_func->bytecode_to_string() << endl;	
	IS_TRUE(dub_func(1,2,3,4).as<int64_t>() == 14);


	// Before the functions are cleaned up they should borrow
	//   references to variables
	IS_TRUE(A.get_refcount() > 1)
	IS_TRUE(B.get_refcount() > 1)
	IS_TRUE(C.get_refcount() > 1)
	IS_TRUE(D.get_refcount() > 1)

	} // END Frame: All funcs should have been deconstructed 

	// Only refrences should be in this stack
	IS_TRUE(A.get_refcount() == 1)
	IS_TRUE(B.get_refcount() == 1)
	IS_TRUE(C.get_refcount() == 1)
	IS_TRUE(D.get_refcount() == 1)
}

void test_basic_str(){
	ref<Var> A = new_var("A", cre_str);
	ref<Var> B = new_var("B", cre_str);
	ref<Var> C = new_var("C", cre_str);
	ref<Var> D = new_var("D", cre_str);

	FuncRef concat_f = define_func<concat>("concat");
	FuncRef paren_f = define_func<paren>("paren");

	Item val;
	// val = concat_f("A","B");
	IS_TRUE(concat_f("A","B").as<std::string>() == "AB");
	// cout << "---------" << endl;
	// cout << val << endl;
	
	FuncRef cat_paren = concat_f(paren_f(A), paren_f(B));
	IS_TRUE(cat_paren("x","y").as<std::string>() == "(x)(y)");
	// cout << "cat_paren: " << cat_paren << endl;
	// val = cat_paren("x","y");
	// cout << "---------" << endl;
	// cout << val << endl;

	FuncRef neg_cat_paren = concat_f("-",paren_f(cat_paren));
	IS_TRUE(neg_cat_paren("x","y").as<std::string>() == "-((x)(y))");
	// cout << neg_cat_paren << endl;
	// val = neg_cat_paren("x","y");
	// cout << "---------" << endl;
	// cout << val << endl;

	// IS_TRUE(cat_paren("x","y") == "(x)(y)");
}

void test_compose_derefs(){
	// FuncRef add_f = define_func("add_flt", (void*) &add, cre_float, {cre_float, cre_float});
	FuncRef add_f = define_func<add_flt>("add_flt");

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
	ref<Fact> rickys_dad = make_fact(Person, "Ricky's Dad", 70.0);
	ref<Fact> ricky = make_fact(Person, "Ricky", 7.11, nullptr, rickys_dad);
	ref<Fact> thedude = make_fact(Person, "TheDude", 23131.73, ma, pops, ricky);

	cout << "Ol'Pops: " << olpops << uint64_t(olpops.get()) << endl;
	cout << "Pops: " << pops << uint64_t(pops.get()) << endl;
	cout << "Ma: " << ma << uint64_t(ma.get()) << endl;
	cout << "Ricky: " << ricky << uint64_t(ricky.get()) <<  endl;
	cout << "TheDude: " << thedude << uint64_t(thedude.get()) <<  endl;
	cout << endl;
	cout << "Pop's Dad: " << pops->get(3) << endl;

	ref<Var> A = new_var("A", Person);
	ref<Var> B = new_var("B", Person);

	ref<Var> Ad = A->extend_attr("dad");
	ref<Var> Bd = B->extend_attr("dad");

	cout << "----------------" << endl;

	FuncRef dadd_m = add_f(Ad->extend_attr("money"), Bd->extend_attr("money"));
	cout << dadd_m << endl;

	// cout << "-------V--------" << endl;
	EXPECT_THROW(dadd_m(pops, olpops));

	// cout << "-------V--------" << endl;
	IS_TRUE(dadd_m(pops, pops) == 7.0);

	cout << "--------------" << endl;
	FuncRef bdadd_m = dadd_m(A->extend_attr("best_bud"), B->extend_attr("best_bud"));
	cout << bdadd_m << endl;

	IS_TRUE(bdadd_m(thedude, thedude) == 140.0);
}


void test_type_check_and_casting(){
	FactType* Person = define_fact("Person", 
	    {{"id", cre_str, {{"unique_id", true}}},
	     {"money", cre_float},
	     {"mom", new DefferedType("Person")},
	     {"dad", new DefferedType("Person")},
	     {"best_bud", new DefferedType("Person")}
	 	}
	);

	FuncRef and_f = define_func<bin_and>("and");
	FuncRef add_f = define_func<add>("add");
	FuncRef add_flt_f = define_func<add_flt>("add_flt");
	FuncRef concat_f = define_func<concat>("concat");
	FuncRef paren_f = define_func<paren>("paren");
	FuncRef add_money_f = define_func<add_money>("add_money");

	ref<Fact> olpops = make_fact(Person, "Ol'Pops", 3.50);
	ref<Fact> pops = make_fact(Person, "Pops", 100.0, nullptr, olpops, nullptr);

	// Always throw an Error on Undefined 
	EXPECT_THROW(and_f(Item(), Item()));
	EXPECT_THROW(add_f(Item(), Item()));
	EXPECT_THROW(add_flt_f(Item(), Item()));
	EXPECT_THROW(concat_f(Item(), Item()));
	EXPECT_THROW(paren_f(Item(), Item()));
	EXPECT_THROW(add_money_f(Item(), Item()));

	// And Bools
	EXPECT_THROW(and_f(olpops, pops));
	EXPECT_THROW(and_f("a", "b"));
	EXPECT_THROW(and_f("a", 1));
	IS_TRUE(and_f(1.0, 1.0) == true);
	IS_TRUE(and_f(true, 3) == true);
	IS_TRUE(and_f(true, false) == false);

	// Add bools as Items
	EXPECT_THROW(and_f(Item(olpops), Item(pops)));
	EXPECT_THROW(and_f(Item("a"), Item("b")));
	EXPECT_THROW(and_f(Item("a"), Item(1)));
	IS_TRUE(and_f(Item(1.0), Item(0.0)) == false);
	IS_TRUE(and_f(Item(true), Item(3)) == true);
	IS_TRUE(and_f(Item(true), Item(false)) == false);

	// Add integers
	EXPECT_THROW(add_f(olpops, pops));
	EXPECT_THROW(add_f("a", "b"));
	EXPECT_THROW(add_f("a", 1));
	IS_TRUE(add_f(1.0, 1.0) == 2);
	IS_TRUE(add_f(true, 3) == 4);

	// Add integers as Items
	EXPECT_THROW(add_f(Item(olpops), Item(pops)));
	EXPECT_THROW(add_f(Item("a"), Item("b")));
	EXPECT_THROW(add_f(Item("a"), Item(1)));
	IS_TRUE(add_f(Item(1.0), Item(1.0)) == 2);
	IS_TRUE(add_f(Item(true), Item(3)) == 4);

	// Add floats
	EXPECT_THROW(add_flt_f(olpops, pops));
	EXPECT_THROW(add_flt_f("a", "b"));
	EXPECT_THROW(add_flt_f("a", 1.7));
	IS_TRUE(add_flt_f(1.8, 1) == 2.8);
	IS_TRUE(add_flt_f(true, 3.7) == 4.7);

	// Add floats as Items
	EXPECT_THROW(add_flt_f(Item(olpops), Item(pops)));
	EXPECT_THROW(add_flt_f(Item("a"), Item("b")));
	EXPECT_THROW(add_flt_f(Item("a"), Item(1)));
	IS_TRUE(add_flt_f(Item(1.8), Item(1)) == 2.8);
	IS_TRUE(add_flt_f(Item(true), Item(3.7)) == 4.7);


	cout << add_money_f(olpops, pops) << endl;


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
	new (stack+1) Item(a);
	new (stack+2) Item(b);
	new (stack) Item(func(stack+1));


	// stack[1] = Item(a);
	// stack[2] = Item(b);
	// stack[0] =
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
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	
	void* args[2] = {stack,stack+8};
	void* ret = &int_stack[2];

	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		int_stack[0] = t>>8;
		int_stack[1] = i;
		int_stack[2] = 0;
		stack_call_func_ptr(add_ptr, ret, args);
		t += int_stack[2];
	} 
	return t;
}

int64_t run_stack_call(){
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	
	// void  args[2] = {stack,stack+8};
	void* args[2] = {stack,stack+8};
	void* ret = &int_stack[2];

	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		int_stack[0] = t>>8;
		int_stack[1] = i;
		int_stack[2] = 0;
		stack_call<add>(ret, args);
		t += int_stack[2];
	} 
	return t;
}

int64_t run_stack_call2(){
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	
	// void  args[2] = {stack,stack+8};
	// void* args[2] = {stack,stack+8};
	void* ret = &int_stack[2];

	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		int_stack[0] = t>>8;
		int_stack[1] = i;
		int_stack[2] = 0;
		stack_call2<add>(ret, stack);
		t += int_stack[2];
	} 
	return t;
}

int64_t run_stack_call_func(auto add_f){
	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	
	void* args[2] = {stack,stack+8};
	void* ret = &int_stack[2];

	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		int_stack[0] = t>>8;
		int_stack[1] = i;
		int_stack[2] = 0;
		add_f->stack_call_func(ret, args);
		t += int_stack[2];
	} 
	return t;
}



int64_t run_call_func(auto add_f){
	// uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	// int64_t* int_stack = (int64_t*) stack;
	
	// void  args[2] = {stack,stack+8};
	// void* args[2] = {stack,stack+8};
	// void* ret = &int_stack[2];

	
	// cout << "Bytecode: " << uint64_t(add_f->bytecode) << endl;

	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		Item val = add_f->call(t>>8, i);

		// int_stack[0] = t>>8;
		// int_stack[1] = i;
		// int_stack[2] = 0;
		// stack_call<add>(ret, args);
		t += val.as<int64_t>();
	} 
	return t;
}


int64_t run_stack_call_generic(){
	// uint16_t arg_offsets[2] = {8,16};
	Item* args = (Item*) alloca(sizeof(Item)*2);
	
	// int64_t* int_stack = (int64_t*) stack;
	int64_t t = 0;
	for(int i=0; i < 1000; ++i){
		// args[0] = Item(0);
		args[0] = Item(t>>8); // <- Extra time is mostly in conversion here
		args[1] = Item(i);
		t += stack_call_generic<add>(args).as<int64_t>();
		// t += int_stack[0];
	} 
	return t;
}

int64_t run_composition(auto add_f){
	// uint16_t arg_offsets[2] = {8,16};
	// FuncRef add_f = define_func<add>("add");


	ref<Var> A = new_var("A", cre_int);
	ref<Var> B = new_var("B", cre_int);
	ref<Var> C = new_var("C", cre_int);

	FuncRef composition = add_f(add_f(add_f(A,B), add_f(2,B)), 7);

	// Item* args = (Item*) alloca(sizeof(Item)*2);


	
	// int64_t* int_stack = (int64_t*) stack;
	int64_t t = 0;
	for(int i=0; i < 1000; ++i){

		t += composition(t>>8, i).as<int64_t>();
		// args[0] = Item(0);
		// args[0] = Item(t>>8); // <- Extra time is mostly in conversion here
		// args[1] = Item(i);
		// t += stack_call_generic<add>(args).as<int64_t>();
		// t += int_stack[0];
	} 
	return t;
}






int main(){
	// FuncRef concat_f = define_func<concat>("concat");
	// call(concat_f, "A","B");
	// cout << "CONCAT:" << concat_f("A","B") << endl;
	// test_basic_int();
	// test_basic_str();
	// test_compose_derefs();
	test_type_check_and_casting();
	return 0;

	// return 0;

	// Using the C++17 fold expression approach
    // constexpr auto offsets_cxx17 = AlignedLayout<char, int, double>::offsets;
    // constexpr auto offsets2 = AlignedStructInfo<char, int, double>::offsets;

    // cout << offsets2 << endl;




    // cout << "Offsets (C++17):" << endl;
    // for (size_t i=0; i < offsets_cxx17.size(); ++i){
    // 	cout << offsets_cxx17[i] << " ";
    // }
    // cout << endl;
    // 	std::size_t offset : offsets_cxx17) {
    // std::cout << offsets_cxx17[0] << " " << std::get<0>(offsets2) << endl;
    // std::cout << offsets_cxx17[1] << " " << std::get<1>(offsets2) << endl;
    // std::cout << offsets_cxx17[2] << " " << std::get<2>(offsets2) << endl;
    // }
    // std::cout << std::endl;
    // std::cout << "Total size (C++17): " << AlignedLayout<char, int, double>::total_size << std::endl;
    // std::cout << "Max alignment (C++17): " << AlignedLayout<char, int, double>::max_align << std::endl;



	// return 0;
	

	int64_t out = 0;

	// time_it_n("run_stack_call_func_ptr", out=run_stack_call_func_ptr(); ,10000);
	// cout << out << endl;

	// time_it_n("run_stack_call_generic", out=run_stack_call_generic(); ,10000);
	// // cout << out << endl;


	FuncRef add_f = define_func<add>("add");
	
	// time_it_n("run_stack_call_func", out=run_stack_call_func(add_f); ,10000);
	// cout << out << endl;

	time_it_n("stack_call<add>", out=run_stack_call(); ,10000);
	cout << out << endl;
	
	time_it_n("run_call_func", out=run_call_func(add_f); ,10000);
	cout << out << endl;

	// time_it_n("stack_call2<add>", out=run_stack_call2(); ,10000);
	// cout << out << endl;

	time_it_n("composition", out=run_composition(add_f); ,10000);
	cout << out << endl;
// 
	return 0;

	/*

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
	*/
	return 0;
}
