// Copyright Louis Dionne 2013-2022
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <boost/hana/equal.hpp>
#include <boost/hana/slice.hpp>
#include <boost/hana/tuple.hpp>
#include <boost/hana/type.hpp>
namespace hana = boost::hana;


static_assert(
    hana::slice_c<1, 3>(hana::make_tuple(1, '2', 3.3, hana::type_c<float>))
            ==
    hana::make_tuple('2', 3.3)
, "");

int main() { }
