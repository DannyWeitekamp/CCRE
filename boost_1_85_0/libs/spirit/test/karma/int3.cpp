//  Copyright (c) 2001-2011 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/spirit/include/karma_int.hpp>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>

#include <boost/spirit/include/karma_char.hpp>
#include <boost/spirit/include/karma_numeric.hpp>
#include <boost/spirit/include/karma_directive.hpp>
#include <boost/spirit/include/karma_action.hpp>
#include <boost/spirit/include/karma_phoenix_attributes.hpp>

#include <boost/limits.hpp>
#include "test.hpp"
#include <sstream>

using namespace spirit_test;

template <typename T>
std::string to_string(T v)
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
struct test_minmax
{
    template <typename T>
    void operator()(T) const
    {
        using namespace boost::spirit;
        using namespace boost::phoenix;

        T minval = (std::numeric_limits<T>::min)();
        T maxval = (std::numeric_limits<T>::max)();

        std::string expected_minval = to_string(minval);
        std::string expected_maxval = to_string(maxval);

        // create a correct generator type from the given integer type
        typedef typename
            boost::mpl::if_<
                boost::mpl::bool_<std::numeric_limits<T>::is_signed>,
                karma::int_generator<T>,
                karma::uint_generator<T>
            >::type
        int_generator_type;

        int_generator_type const gen = int_generator_type();

        BOOST_TEST(test(expected_maxval, gen, maxval));
        BOOST_TEST(test(expected_minval, gen, minval));
        BOOST_TEST(test(expected_maxval, gen(maxval)));
        BOOST_TEST(test(expected_minval, gen(minval)));
        BOOST_TEST(test(expected_maxval, gen(maxval), maxval));
        BOOST_TEST(test(expected_minval, gen(minval), minval));
        BOOST_TEST(!test("", gen(maxval), maxval-1));
        BOOST_TEST(!test("", gen(minval), minval+1));
        BOOST_TEST(test(expected_maxval, lit(maxval)));
        BOOST_TEST(test(expected_minval, lit(minval)));

        BOOST_TEST(test_delimited(expected_maxval + " ", gen, maxval, char(' ')));
        BOOST_TEST(test_delimited(expected_minval + " ", gen, minval, char(' ')));
        BOOST_TEST(test_delimited(expected_maxval + " ", gen(maxval), char(' ')));
        BOOST_TEST(test_delimited(expected_minval + " ", gen(minval), char(' ')));
        BOOST_TEST(test_delimited(expected_maxval + " ", gen(maxval), maxval, char(' ')));
        BOOST_TEST(test_delimited(expected_minval + " ", gen(minval), minval, char(' ')));
        BOOST_TEST(!test_delimited("", gen(maxval), maxval-1, char(' ')));
        BOOST_TEST(!test_delimited("", gen(minval), minval+1, char(' ')));
        BOOST_TEST(test_delimited(expected_maxval + " ", lit(maxval), char(' ')));
        BOOST_TEST(test_delimited(expected_minval + " ", lit(minval), char(' ')));

    // action tests
        BOOST_TEST(test(expected_maxval, gen[_1 = val(maxval)]));
        BOOST_TEST(test(expected_minval, gen[_1 = val(minval)]));

    // optional tests
        boost::optional<T> optmin, optmax(maxval);

        BOOST_TEST(!test("", gen, optmin));
        BOOST_TEST(!test("", gen(minval), optmin));

        optmin = minval;
        BOOST_TEST(test(expected_minval, gen, optmin));
        BOOST_TEST(test(expected_maxval, gen, optmax));
        BOOST_TEST(test(expected_minval, gen(minval), optmin));
        BOOST_TEST(test(expected_maxval, gen(maxval), optmax));

    // Phoenix expression tests (only supported while including
    // karma_phoenix_attributes.hpp
        namespace phoenix = boost::phoenix;

        BOOST_TEST(test("1", gen, phoenix::val(1)));

        T val = 1;
        BOOST_TEST(test("1", gen, phoenix::ref(val)));
        BOOST_TEST(test("2", gen, ++phoenix::ref(val)));
    }
};

///////////////////////////////////////////////////////////////////////////////
int
main()
{
    using namespace boost::spirit;

// test boundary values
    typedef boost::mpl::vector<
#ifdef BOOST_HAS_LONG_LONG
        boost::long_long_type, boost::ulong_long_type,
#endif
        short, unsigned short, 
        int, unsigned int, 
        long, unsigned long
    > integer_types;
    boost::mpl::for_each<integer_types>(test_minmax());

    return boost::report_errors();
}

