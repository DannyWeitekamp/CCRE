//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2012. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/interprocess/detail/workaround.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "mutex_test_template.hpp"

int main ()
{
   using namespace boost::interprocess;

   test::test_all_lock<interprocess_mutex>();
   test::test_all_mutex<interprocess_mutex>();
   return 0;
}
