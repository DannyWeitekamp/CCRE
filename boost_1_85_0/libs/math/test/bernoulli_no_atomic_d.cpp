//  (C) Copyright John Maddock 2021.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_MATH_NO_ATOMIC_INT

#include <boost/math/special_functions/bernoulli.hpp>
#include "compile_test/test_compile_result.hpp"

void compile_and_link_test()
{
   check_result<double>(boost::math::bernoulli_b2n<double>(4));
}
