#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <nanobind/nanobind.h>
#include "../../include/cre_obj.h"
#include "../../include/ref.h"

#include "../../include/context.h"
#include "../../include/types.h"
#include "../../include/intern.h"
#include "../../include/item.h"
#include "../../include/fact.h"

#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/make_iterator.h>
#include <nanobind/ndarray.h>

namespace nb = nanobind;
using namespace nb::literals;
using std::cout;
using std::endl;

// Defined in py_cre_core.cpp
Item Item_from_py(nb::handle py_obj);
nb::object Item_to_py(Item item);
std::string Type_name_from_py(nb::handle py_obj);
FactType* FactType_from_py(nb::handle py_obj);
CRE_Type* Type_from_py(nb::handle py_obj);
FlagGroup dict_to_flag_group(nb::dict py_dict);
std::vector<FlagGroup> py_to_flag_groups(nb::handle py_obj);

ref<Fact> _py_new_fact(
	FactType* type, 
    int n_pos_args,
    nb::detail::fast_iterator args_start,
    nb::detail::fast_iterator args_end,
    nb::kwargs kwargs,
    bool immutable=false
);

void cre_obj_dealloc(PyObject *self);

extern PyType_Slot cre_obj_slots[];
