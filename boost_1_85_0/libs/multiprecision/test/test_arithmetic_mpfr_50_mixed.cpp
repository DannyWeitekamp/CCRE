///////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <boost/multiprecision/mpfr.hpp>
#define TEST_MPFR
#include "test_arithmetic.hpp"

template <>
struct related_type<boost::multiprecision::static_mpfr_float_50>
{
   typedef boost::multiprecision::mpfr_float_50 type;
};

int main()
{
   test<boost::multiprecision::static_mpfr_float_50>();
   return boost::report_errors();
}
