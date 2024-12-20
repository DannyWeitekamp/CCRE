///////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/debug_adaptor.hpp>

#include "test_arithmetic.hpp"

int main()
{
   test<boost::multiprecision::number<boost::multiprecision::debug_adaptor<typename boost::multiprecision::cpp_complex_50::backend_type> > >();
   return boost::report_errors();
}
