// Copyright Louis Dionne 2013-2022
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <boost/hana/assert.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/plus.hpp>
#include <boost/hana/string.hpp>
namespace hana = boost::hana;


auto hello_world = BOOST_HANA_STRING("Hello ") + BOOST_HANA_STRING("world!");
BOOST_HANA_CONSTANT_CHECK(hello_world == BOOST_HANA_STRING("Hello world!"));

int main() { }
