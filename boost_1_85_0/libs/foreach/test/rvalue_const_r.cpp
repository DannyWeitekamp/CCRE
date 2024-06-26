//  (C) Copyright Eric Niebler 2005.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/*
  Revision history:
  25 August 2005 : Initial version.
*/

#include <vector>
#include <boost/core/lightweight_test.hpp>
#include <boost/foreach.hpp>

#ifdef BOOST_FOREACH_NO_CONST_RVALUE_DETECTION
// ignore error during Microsoft Code Analysis
#if !defined(_PREFAST_)
# error Expected failure : const rvalues disallowed
#endif
#else

std::vector<int> const get_vector()
{
    return std::vector<int>(4, 4);
}

///////////////////////////////////////////////////////////////////////////////
// test_main
//   
int main()
{
    int counter = 0;

    BOOST_REVERSE_FOREACH(int i, get_vector())
    {
        counter += i;
    }

    BOOST_TEST(16 == counter);

    return boost::report_errors();
}

#endif
