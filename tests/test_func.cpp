#include "../include/helpers.h"
#include "../include/func.h"
#include "../include/item.h"
#include "test_macros.h"
#include <random>

#include <fmt/format.h>
#include <fmt/ranges.h>

using std::cout;
using std::endl;

template<typename func>
struct test_func {
	Item& operator()(Item&, ...){

	}
};

int64_t add(int64_t a, int64_t b){
	return a+b;
}

typedef int64_t (*two_int_func_t)(int64_t a, int64_t b);
two_int_func_t add_ptr = &add;

void test_define(){
	ref<Func> func = define_func("add", (void*) &add, cre_int, {cre_int, cre_int});
	cout << func << endl;

	func->set_arg(0,1);
	cout << func << endl;
	func->reinitialize();
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

void add_ptrs(uint8_t* ret, uint8_t* args){
	int64_t a = *((int64_t*) &args[0]);
	int64_t b = *((int64_t*) &args[8]);
	*((int64_t*) ret) = a+b;
	// return a+b;
}

typedef void (*abs_func_t)(uint8_t* ret, uint8_t* args);
abs_func_t add_ptrs_ptr = &add_ptrs;



int main(){
	// test_define();

	auto func = (*add_items_ptr);

	time_it_n("inline", add(1,2), 100000);
	time_it_n("opaque_ptr", (*add_ptr)(1,2) ,100000);
	time_it_n("item_alloca", add_alloca(1,2) ,100000);
	time_it_n("add_item_no_stack", add_item_no_stack(1,2) ,100000);

	uint8_t* stack = (uint8_t*) alloca(sizeof(int64_t)*3);
	int64_t* int_stack = (int64_t*) stack;
	int_stack[1] = 1;
	int_stack[2] = 2;
	time_it_n("add_abs_extern_stack", add_ptrs(stack, &stack[1]) ,100000);

	return 0;
}
