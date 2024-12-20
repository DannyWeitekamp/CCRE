
// Copyright 2006-2009 Daniel James.
// Copyright (C) 2022-2023 Christian Mazakas
// Copyright (C) 2024 Joaquin M Lopez Munoz.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "../helpers/unordered.hpp"

#include "../helpers/test.hpp"
#include "../objects/test.hpp"
#include "../helpers/random_values.hpp"
#include "../helpers/tracker.hpp"
#include "../helpers/helpers.hpp"

namespace find_tests {

  test::seed_t initialize_seed(78937);

  template <class X> void find_tests1(X*, test::random_generator generator)
  {
    typedef typename X::iterator iterator;

    {
      test::check_instances check_;

      test::random_values<X> v(500, generator);
      X x(v.begin(), v.end());
      X const& x_const = x;
      test::ordered<X> tracker = test::create_ordered(x);
      tracker.insert_range(v.begin(), v.end());

      for (typename test::ordered<X>::const_iterator it1 = tracker.begin();
           it1 != tracker.end(); ++it1) {
        typename X::key_type key = test::get_key<X>(*it1);
        typename X::const_iterator const_pos = x_const.find(key);
        iterator pos = x.find(key);
        BOOST_TEST(const_pos != x_const.end());
        BOOST_TEST(const_pos != x_const.end() &&
                   x_const.key_eq()(key, test::get_key<X>(*const_pos)));
        BOOST_TEST(pos != x.end());
        BOOST_TEST(pos != x.end() && x.key_eq()(key, test::get_key<X>(*pos)));

        BOOST_TEST(x.count(key) == tracker.count(key));

        test::compare_pairs(x.equal_range(key), tracker.equal_range(key),
          (typename X::value_type*)0);
        test::compare_pairs(x_const.equal_range(key), tracker.equal_range(key),
          (typename X::value_type*)0);
      }

      test::random_values<X> v2(500, generator);
      for (typename test::random_values<X>::const_iterator it2 = v2.begin();
           it2 != v2.end(); ++it2) {
        typename X::key_type key = test::get_key<X>(*it2);
        if (tracker.find(test::get_key<X>(key)) == tracker.end()) {
          BOOST_TEST(x.find(key) == x.end());
          BOOST_TEST(x_const.find(key) == x_const.end());
          BOOST_TEST(x.count(key) == 0);
          std::pair<iterator, iterator> range = x.equal_range(key);
          BOOST_TEST(range.first == range.second);
        }
      }
    }

    {
      test::check_instances check_;

      X x;

      test::random_values<X> v2(5, generator);
      for (typename test::random_values<X>::const_iterator it3 = v2.begin();
           it3 != v2.end(); ++it3) {
        typename X::key_type key = test::get_key<X>(*it3);
        BOOST_TEST(x.find(key) == x.end());
        BOOST_TEST(x.count(key) == 0);
        std::pair<iterator, iterator> range = x.equal_range(key);
        BOOST_TEST(range.first == range.second);
      }
    }
  }

  struct compatible_key
  {
    test::object o_;

    compatible_key(test::object const& o) : o_(o) {}
  };

  struct compatible_hash
  {
    test::hash hash_;

    std::size_t operator()(compatible_key const& k) const
    {
      return hash_(k.o_);
    }
  };

  struct compatible_predicate
  {
    test::equal_to equal_;

    bool operator()(compatible_key const& k1, compatible_key const& k2) const
    {
      return equal_(k1.o_, k2.o_);
    }
  };

  template <class X>
  void find_compatible_keys_test(X*, test::random_generator generator)
  {
    typedef typename test::random_values<X>::iterator value_iterator;
    test::random_values<X> v(500, generator);
    X x(v.begin(), v.end());
    X const& x_const = x;

    compatible_hash h;
    compatible_predicate eq;

    for (value_iterator it = v.begin(), end = v.end(); it != end; ++it) {
      typename X::key_type key = test::get_key<X>(*it);
      BOOST_TEST(x.find(key) == x.find(compatible_key(key), h, eq));
      BOOST_TEST(x_const.find(key) == x_const.find(compatible_key(key), h, eq));
    }

    test::random_values<X> v2(20, generator);

    for (value_iterator it = v2.begin(), end = v2.end(); it != end; ++it) {
      typename X::key_type key = test::get_key<X>(*it);
      BOOST_TEST(x.find(key) == x.find(compatible_key(key), h, eq));
      BOOST_TEST(x_const.find(key) == x_const.find(compatible_key(key), h, eq));
    }
  }

  using test::default_generator;
  using test::generate_collisions;
  using test::limited_range;

#ifdef BOOST_UNORDERED_FOA_TESTS
  boost::unordered_flat_set<test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_set;
  boost::unordered_flat_map<test::object, test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_map;
  boost::unordered_node_set<test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_node_set;
  boost::unordered_node_map<test::object, test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_node_map;

  UNORDERED_TEST(
    find_tests1, ((test_set)(test_map)(test_node_set)(test_node_map))(
                   (default_generator)(generate_collisions)(limited_range)))
#else
  boost::unordered_set<test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_set;
  boost::unordered_multiset<test::object, test::hash, test::equal_to,
    test::allocator1<test::object> >* test_multiset;
  boost::unordered_map<test::object, test::object, test::hash, test::equal_to,
    test::allocator2<test::object> >* test_map;
  boost::unordered_multimap<test::object, test::object, test::hash,
    test::equal_to, test::allocator1<test::object> >* test_multimap;

  UNORDERED_TEST(
    find_tests1, ((test_set)(test_multiset)(test_map)(test_multimap))(
                   (default_generator)(generate_collisions)(limited_range)))
  UNORDERED_TEST(find_compatible_keys_test,
    ((test_set)(test_multiset)(test_map)(test_multimap))(
      (default_generator)(generate_collisions)(limited_range)))
#endif
}

RUN_TESTS()
