// Copyright Louis Dionne 2013-2022
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#include <boost/hana/assert.hpp>
#include <boost/hana/empty.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/optional.hpp>
namespace hana = boost::hana;


int main() {
    BOOST_HANA_CONSTANT_CHECK(hana::equal(
        hana::empty<hana::optional_tag>(),
        hana::nothing
    ));
}
