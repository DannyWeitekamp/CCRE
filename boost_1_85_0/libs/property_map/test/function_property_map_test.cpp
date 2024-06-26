//
//=======================================================================
// Author: Jeremiah Willcock
//
// Copyright 2012, Trustees of Indiana University
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

#include <boost/property_map/function_property_map.hpp>
#include <boost/concept/assert.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/static_assert.hpp>

template <typename T>
struct add1 {typedef T result_type; T operator()(const T& x) const {return x + 1;}};

template <typename T>
struct add1_val {typedef T result_type; T operator()(T x) const {return x + 1;}};

template <typename T>
struct return_fixed_ref {
  int* ptr;
  return_fixed_ref(int* ptr): ptr(ptr) {}
  typedef int& result_type;
  int& operator()(const T&) const {return *ptr;}
};

int main() {
  using namespace boost;
  BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept<function_property_map<add1<int>, int>, int>));
  BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept<function_property_map<add1<int>, int, double>, int>));
  BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept<function_property_map<add1_val<int>, int>, int>));
  BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept<function_property_map<add1_val<int>, int, double>, int>));
  BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept<function_property_map<return_fixed_ref<int>, int>, int>));
  BOOST_CONCEPT_ASSERT((WritablePropertyMapConcept<function_property_map<return_fixed_ref<int>, int>, int>));
  BOOST_CONCEPT_ASSERT((ReadWritePropertyMapConcept<function_property_map<return_fixed_ref<int>, int>, int>));
  BOOST_CONCEPT_ASSERT((LvaluePropertyMapConcept<function_property_map<return_fixed_ref<int>, int>, int>));

  BOOST_STATIC_ASSERT((boost::is_same<boost::property_traits<function_property_map<add1<int>, int> >::category, boost::readable_property_map_tag>::value));
  BOOST_STATIC_ASSERT((boost::is_same<boost::property_traits<function_property_map<add1_val<int>, int> >::category, boost::readable_property_map_tag>::value));
  BOOST_STATIC_ASSERT((boost::is_same<boost::property_traits<function_property_map<return_fixed_ref<int>, int> >::category, boost::lvalue_property_map_tag>::value));

  BOOST_TEST(get(function_property_map<add1<int>, int>(), 3) == 4);
  BOOST_TEST(get(function_property_map<add1<int>, int>(add1<int>()), 4) == 5);
  BOOST_TEST(get(make_function_property_map<int>(add1<int>()), 5) == 6);
  BOOST_TEST(get(function_property_map<add1_val<int>, int>(), 3) == 4);
  BOOST_TEST(get(function_property_map<add1_val<int>, int>(add1_val<int>()), 4) == 5);
  BOOST_TEST(get(make_function_property_map<int>(add1_val<int>()), 5) == 6);
  int val;
  const function_property_map<return_fixed_ref<int>, int> pm = return_fixed_ref<int>((&val));
  put(pm, 1, 6);
  BOOST_TEST(get(pm, 2) == 6);
  BOOST_TEST((get(pm, 3) = 7) == 7);
  BOOST_TEST(get(pm, 4) == 7);
  const function_property_map<return_fixed_ref<int>, int> pm2 = pm; // Check shallow copying
  BOOST_TEST(get(pm2, 5) == 7);
  put(pm2, 3, 1);
  BOOST_TEST(get(pm, 1) == 1);

  return boost::report_errors();
}
