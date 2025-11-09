
#include "../include/py_cre_core.h"


Item py_Func_call_to_item(Func* func, nb::args args, nb::kwargs kwargs);
ref<Func> py_Func_compose(Func* func, nb::args args);
CRE_Obj* py_to_cre_obj(nb::handle py_obj);
void* py_resolve_heads(void* dest, nb::object py_obj, const HeadInfo& hi);

