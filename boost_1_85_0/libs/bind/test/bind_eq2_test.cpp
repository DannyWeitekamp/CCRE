#include <boost/config.hpp>

#if defined(BOOST_MSVC)
#pragma warning(disable: 4786)  // identifier truncated in debug info
#pragma warning(disable: 4710)  // function not inlined
#pragma warning(disable: 4711)  // function selected for automatic inline expansion
#pragma warning(disable: 4514)  // unreferenced inline removed
#endif

//
//  bind_eq2_test.cpp - boost::bind equality operator
//
//  Copyright (c) 2004, 2005, 2009 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/bind/bind.hpp>
#include <boost/function_equal.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::placeholders;

//

void f( int )
{
}

int g( int i )
{
    return i + 5;
}

template< class F > void test_self_equal( F f )
{
#ifdef BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP
    using boost::function_equal;
#endif

    BOOST_TEST( function_equal( f, f ) );
}
 
int main()
{
    test_self_equal( boost::bind( f, _1 ) );
    test_self_equal( boost::bind( g, _1 ) );
    test_self_equal( boost::bind( f, boost::bind( g, _1 ) ) );

    return boost::report_errors();
}
