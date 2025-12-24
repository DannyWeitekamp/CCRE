#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <nanobind/nanobind.h>
#include "../../include/cre_obj.h"
#include "../../include/ref.h"
#include "../../external/fmt/include/fmt/format.h"

#include "../../include/context.h"
#include "../../include/types.h"
#include "../../include/intern.h"
#include "../../include/item.h"
#include "../../include/fact.h"
#include "../../include/var.h"
#include "../../include/func.h"
#include "../../include/literal.h"
#include "../../include/logic.h"

#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>
#include <nanobind/make_iterator.h>
#include <nanobind/ndarray.h>

namespace nb = nanobind;
using namespace nb::literals;
using std::cout;
using std::endl;

using namespace cre;


// Defined in py_cre_core.cpp
Item Item_from_py(nb::handle py_obj);
nb::object Item_to_py(Item item);
std::string Type_name_from_py(nb::handle py_obj);
FactType* FactType_from_py(nb::handle py_obj);
CRE_Type* Type_from_py(nb::handle py_obj);
CRE_Type* py_cls_to_CRE_Type(nb::handle py_obj);
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

inline nb::handle _extract_arg_or_kwarg(std::string_view arg_name, size_t arg_index, nb::args args, nb::kwargs kwargs){
    nb::handle obj = kwargs.get(arg_name.data(), nb::none());
    if(obj.is_none() && args.size() >= arg_index+1){
        obj = args[arg_index];
    }
    return obj;
}


template<typename T>
T resolve_arg_or_kwarg(std::string_view fn_name, std::string_view arg_name, size_t arg_index, T&& default_value, nb::args args, nb::kwargs kwargs){
    using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;
    nb::handle obj = _extract_arg_or_kwarg(arg_name, arg_index, args, kwargs);
    if(obj.is_none()){
        return std::forward<T>(default_value);
    }
    if(!nb::isinstance<T>(obj)){
        throw std::runtime_error(fmt::format("{}: '{}' argument must be a {}, but got: {}", fn_name, arg_name, type_name_helper<DecayT>(), nb::cast<std::string>(nb::str(obj))));
    }
    return nb::cast<T>(obj);
}

template<typename T>
T require_arg_or_kwarg(std::string_view fn_name, std::string_view arg_name, size_t arg_index, nb::args args, nb::kwargs kwargs){
    using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;
    nb::handle obj = _extract_arg_or_kwarg(arg_name, arg_index, args, kwargs);
    if(obj.is_none()){
        throw std::runtime_error(fmt::format("{}: requires at least '{}' argument as positional arg={} or keyword arg='{}'", fn_name, arg_name, arg_index, arg_name));
    }
    if(!nb::isinstance<T>(obj)){
        throw std::runtime_error(fmt::format("{}: '{}' argument must be a {}, but got: {}", fn_name, arg_name, type_name_helper<DecayT>(), nb::cast<std::string>(nb::str(obj))));
    }
    return nb::cast<T>(obj);
}

extern PyType_Slot cre_obj_slots[];
