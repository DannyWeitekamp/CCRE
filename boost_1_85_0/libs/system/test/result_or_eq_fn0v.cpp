// Copyright 2017, 2021-2024 Peter Dimov.
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/system/result.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost::system;

struct X
{
    int v_;
};

struct Y
{
    int v_;

    explicit Y( int v ): v_( v ) {}
    Y( X x ): v_( x.v_) {}

    Y( Y const& ) = delete;
    Y& operator=( Y const& ) = delete;

    Y( Y&& r ): v_( r.v_ )
    {
        r.v_ = 0;
    }

    Y& operator=( Y&& r )
    {
        if( &r != this )
        {
            v_ = r.v_;
            r.v_ = 0;
        }

        return *this;
    }
};

struct E
{
};

int f()
{
    return 2;
}

X g()
{
    return { 2 };
}

int& h()
{
    static int x = 2;
    return x;
}

int main()
{
    {
        result<int> r( 1 );

        r |= f;

        BOOST_TEST( r.has_value() ) && BOOST_TEST_EQ( *r, 1 );
    }

    {
        result<int, E> r( in_place_error );

        r |= f;

        BOOST_TEST( r.has_value() ) && BOOST_TEST_EQ( *r, 2 );
    }

    {
        result<Y> r( in_place_value, 1 );

        r |= g;

        BOOST_TEST( r.has_value() ) && BOOST_TEST_EQ( r->v_, 1 );
    }

    {
        result<Y, E> r( in_place_error );

        r |= g;

        BOOST_TEST( r.has_value() ) && BOOST_TEST_EQ( r->v_, 2 );
    }

    {
        int x1 = 1;

        result<int&> r( x1 );

        r |= h;

        BOOST_TEST( r.has_value() ) && BOOST_TEST_EQ( &*r, &x1 );
    }

    {
        result<int&, E> r( in_place_error );

        r |= h;

        BOOST_TEST( r.has_value() ) && BOOST_TEST_EQ( &*r, &h() );
    }

    return boost::report_errors();
}
