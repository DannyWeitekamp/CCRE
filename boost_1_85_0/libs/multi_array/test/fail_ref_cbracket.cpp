// Copyright 2002 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software 
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Boost.MultiArray Library
//  Authors: Ronald Garcia
//           Jeremy Siek
//           Andrew Lumsdaine
//  See http://www.boost.org/libs/multi_array for documentation.

// 
// fail_ref_cbracket.cpp
//   checking constness of  const operator[].
//

#include <boost/multi_array.hpp>

#include <boost/core/lightweight_test.hpp>

#include <boost/array.hpp>

int
main()
{
  const int ndims=3;
  typedef boost::multi_array_ref<int,ndims> array_ref;

  boost::array<array_ref::size_type,ndims> sma_dims = {{2,3,4}};

  int data[] = {77,77,77,77,77,77,77,77,77,77,77,77, 
                 77,77,77,77,77,77,77,77,77,77,77,77}; 

  array_ref sma(data,sma_dims);
  

  const array_ref& csma = sma;

  // FAIL! can't assign to const multi_array_ref.
  csma[0][0][0] = 5;

  return boost::report_errors();
}
