//  Copyright Matt Borland 2021.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// A sanity check that this file
// #includes all the files that it needs to.
//
#if __has_include(<fftw3.h>)

#include <boost/math/interpolators/cardinal_trigonometric.hpp>
//
// Note this header includes no other headers, this is
// important if this test is to be meaningful:
//
#include "test_compile_result.hpp"
//
// This test includes <vector> because many of the interpolators are not compatible with pointers/c-style arrays
//
#include <vector>

void compile_and_link_test()
{
   std::vector<double> data = { 1, 2, 3 };
   boost::math::interpolators::cardinal_trigonometric<std::vector<double>> s(data, 3, 2);
   check_result<double>(s(1.0));
}
#else
void compile_and_link_test() {}
#endif // __has_include
