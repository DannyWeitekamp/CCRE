
//  (C) Copyright John Maddock 2005. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef TEST_STD
#  include <type_traits>
#else
#  include <boost/type_traits/is_floating_point.hpp>
#endif
#include "test.hpp"
#include "check_integral_constant.hpp"

#ifndef BOOST_NO_CXX23_HDR_STDFLOAT
#include <stdfloat>
#endif

TT_TEST_BEGIN(is_floating_point)

BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float const>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float volatile>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float const volatile>::value, true);

BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<double>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<double const>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<double volatile>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<double const volatile>::value, true);

BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<long double>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<long double const>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<long double volatile>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<long double const volatile>::value, true);

#ifdef BOOST_HAS_FLOAT128
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<__float128>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const __float128>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<volatile __float128>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const volatile __float128>::value, true);
#endif
#ifndef BOOST_NO_CXX23_HDR_STDFLOAT
#if defined(__STDCPP_FLOAT16_T__)
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<std::float16_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const std::float16_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<volatile std::float16_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const volatile std::float16_t>::value, true);
#endif
#if defined(__STDCPP_FLOAT32_T__)
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<std::float32_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const std::float32_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<volatile std::float32_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const volatile std::float32_t>::value, true);
#endif
#if defined(__STDCPP_FLOAT64_T__)
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<std::float32_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const std::float32_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<volatile std::float32_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const volatile std::float32_t>::value, true);
#endif
#if defined(__STDCPP_FLOAT128_T__)
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<std::float128_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const std::float128_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<volatile std::float128_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const volatile std::float128_t>::value, true);
#endif
#if defined(__STDCPP_BFLOAT16_T__)
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<std::bfloat16_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const std::bfloat16_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<volatile std::bfloat16_t>::value, true);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const volatile std::bfloat16_t>::value, true);
#endif
#endif

//
// cases that should not be true:
//
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<void>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<int>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<UDT>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<test_abc1>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<empty_UDT>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float*>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float&>::value, false);
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float&&>::value, false);
#endif
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<const float&>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<float[2]>::value, false);

//
// tricky cases:
//
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<test_abc1>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<foo0_t>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<foo1_t>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<foo2_t>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<foo3_t>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<foo4_t>::value, false);
BOOST_CHECK_INTEGRAL_CONSTANT(::tt::is_floating_point<incomplete_type>::value, false);

TT_TEST_END








