//  Copyright Matt Borland 2021.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Basic sanity check that header
// #includes all the files that it needs to.
//
#include <boost/math/distributions/empirical_cumulative_distribution_function.hpp>
//
// Note this header includes no other headers, this is
// important if this test is to be meaningful:
//
#include "test_compile_result.hpp"

void compile_and_link_test()
{
    boost::math::empirical_cumulative_distribution_function<float[]> f_test(float[]);
    boost::math::empirical_cumulative_distribution_function<double[]> d_test(double[]);
    #ifndef BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
    boost::math::empirical_cumulative_distribution_function<long double[]> d_test(long double[]);
    #endif
}
