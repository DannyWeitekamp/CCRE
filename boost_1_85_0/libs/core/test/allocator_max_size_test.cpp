/*
Copyright 2020 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#include <boost/core/allocator_access.hpp>
#include <boost/core/lightweight_test.hpp>
#include <limits>

template<class T>
struct A1 {
    typedef T value_type;
    typedef short size_type;
    A1() { }
    short max_size() const {
        return 1;
    }
};

template<class T>
struct A2 {
    typedef T value_type;
    typedef short size_type;
    A2() { }
};

int main()
{
    BOOST_TEST_EQ(boost::allocator_max_size(A1<int>()), 1);
    BOOST_TEST_LE(boost::allocator_max_size(A2<int>()),
        (std::numeric_limits<short>::max)());
    return boost::report_errors();
}
