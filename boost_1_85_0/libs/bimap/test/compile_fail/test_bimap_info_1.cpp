// Boost.Bimap
//
// Copyright (c) 2006-2007 Matias Capeletto
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  VC++ 8.0 warns on usage of certain Standard Library and API functions that
//  can be cause buffer overruns or other possible security issues if misused.
//  See https://web.archive.org/web/20071014014301/http://msdn.microsoft.com/msdnmag/issues/05/05/SafeCandC/default.aspx
//  But the wording of the warning is misleading and unsettling, there are no
//  portable alternative functions, and VC++ 8.0's own libraries use the
//  functions in question. So turn off the warnings.
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE

#include <boost/config.hpp>

// Boost.Bimap
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/list_of.hpp>


void test_bimap_info_1()
{
    using namespace boost::bimaps;

    typedef bimap< int, list_of<int>, with_info<int> > bm_type;
    bm_type bm;
    bm.insert( bm_type::value_type(1,1) );

    // fail test
    {
        const bm_type & cbm = bm;
        cbm.begin()->info = 10;
    }
}
