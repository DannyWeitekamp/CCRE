//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//  Adaptation to Boost of the libcxx
//  Copyright 2010 Vicente J. Botet Escriba
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

// test ratio:  The static data members num and den shall have thcommon
//    divisor of the absolute values of N and D:

#include <boost/ratio/ratio.hpp>

#define STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

template <long long N, long long D, long long eN, long long eD>
void test()
{
    STATIC_ASSERT(boost::ratio<N, D>::num == eN);
    STATIC_ASSERT(boost::ratio<N, D>::den == eD);
}

int main()
{
    test<1, 1, 1, 1>();
    test<1, 10, 1, 10>();
    test<10, 10, 1, 1>();
    test<10, 1, 10, 1>();
    test<12, 4, 3, 1>();
    test<12, -4, -3, 1>();
    test<-12, 4, -3, 1>();
    test<-12, -4, 3, 1>();
    test<4, 12, 1, 3>();
    test<4, -12, -1, 3>();
    test<-4, 12, -1, 3>();
    test<-4, -12, 1, 3>();
    test<222, 333, 2, 3>();
    test<222, -333, -2, 3>();
    test<-222, 333, -2, 3>();
    test<-222, -333, 2, 3>();
    //test<BOOST_RATIO_INTMAX_T_MAX, 127, 72624976668147841LL, 1>();
    //test<-BOOST_RATIO_INTMAX_T_MAX, 127, -72624976668147841LL, 1>();
    //test<BOOST_RATIO_INTMAX_T_MAX, -127, -72624976668147841LL, 1>();
    //test<-BOOST_RATIO_INTMAX_T_MAX, -127, 72624976668147841LL, 1>();
    //~ test<BOOST_RATIO_INTMAX_T_MAX, 127, BOOST_RATIO_INTMAX_T_MAX, 127>();
    //~ test<-BOOST_RATIO_INTMAX_T_MAX, 127, -BOOST_RATIO_INTMAX_T_MAX, 127>();
    //~ test<BOOST_RATIO_INTMAX_T_MAX, -127, -BOOST_RATIO_INTMAX_T_MAX, 127>();
    //~ test<-BOOST_RATIO_INTMAX_T_MAX, -127, BOOST_RATIO_INTMAX_T_MAX, 127>();
}
