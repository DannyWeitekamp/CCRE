///////////////////////////////////////////////////////////////
//  Copyright 2012 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#include <boost/multiprecision/cpp_int.hpp>

#include "test_arithmetic.hpp"

template <std::size_t MinBits, std::size_t MaxBits, boost::multiprecision::cpp_integer_type SignType, class Allocator, boost::multiprecision::expression_template_option ET>
struct is_checked_cpp_int<boost::multiprecision::number<boost::multiprecision::cpp_int_backend<MinBits, MaxBits, SignType, boost::multiprecision::checked, Allocator>, ET> > : public std::integral_constant<bool, true>
{};

template <std::size_t MinBits, std::size_t MaxBits, boost::multiprecision::cpp_integer_type SignType, class Allocator, boost::multiprecision::expression_template_option ExpressionTemplates>
struct is_twos_complement_integer<boost::multiprecision::number<boost::multiprecision::cpp_int_backend<MinBits, MaxBits, SignType, boost::multiprecision::checked, Allocator>, ExpressionTemplates> > : public std::integral_constant<bool, false>
{};

template <>
struct related_type<boost::multiprecision::cpp_int>
{
   typedef boost::multiprecision::int256_t type;
};
template <std::size_t MinBits, std::size_t MaxBits, boost::multiprecision::cpp_integer_type SignType, boost::multiprecision::cpp_int_check_type Checked, class Allocator, boost::multiprecision::expression_template_option ET>
struct related_type<boost::multiprecision::number<boost::multiprecision::cpp_int_backend<MinBits, MaxBits, SignType, Checked, Allocator>, ET> >
{
   typedef boost::multiprecision::number<boost::multiprecision::cpp_int_backend<MinBits / 2, MaxBits / 2, SignType, Checked, Allocator>, ET> type;
};

int main()
{
   test<boost::multiprecision::number<boost::multiprecision::cpp_int_backend<500, 500, boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> > >();
   return boost::report_errors();
}
