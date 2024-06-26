//
// Copyright 2022 Marco Langer <langer.m86 at gmail dot com>
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
#include <boost/gil/extension/dynamic_image/algorithm.hpp>
#include <boost/gil/extension/dynamic_image/any_image.hpp>

#include <boost/core/lightweight_test.hpp>

#include <utility>

#include "core/image/test_fixture.hpp"
#include "extension/dynamic_image/test_fixture.hpp"

namespace gil = boost::gil;
namespace fixture = boost::gil::test::fixture;

struct test_fill_pixels
{
    template <typename ImageLhs, typename ImageRhs>
    void operator()(std::pair<ImageLhs, ImageRhs> const&)
    {
        using image_lhs_t = ImageLhs;
        using image_rhs_t = ImageRhs;
        using image_view_lhs_t = typename image_lhs_t::view_t;
        using image_view_rhs_t = typename image_rhs_t::view_t;
        using value_lhs_t = typename image_lhs_t::value_type;
        using value_rhs_t = typename image_rhs_t::value_type;

        image_lhs_t image_lhs = fixture::create_image<image_lhs_t>(2, 2, 128);
        fixture::dynamic_image dyn_image_lhs1(image_lhs);
        fixture::dynamic_image dyn_image_lhs2(image_lhs_t(2, 2));
        fixture::dynamic_image dyn_image_lhs3(image_lhs_t(2, 2));
        value_lhs_t value_lhs(128);
        value_rhs_t value_rhs(128);

        // lhs, lhs
        BOOST_TEST_NO_THROW(
            gil::fill_pixels(
                gil::view(dyn_image_lhs2), value_lhs));
        BOOST_TEST(
            gil::equal_pixels(
                gil::const_view(dyn_image_lhs1),
                gil::const_view(dyn_image_lhs2)));

        // lhs, rhs
        if (gil::views_are_compatible<image_view_lhs_t, image_view_rhs_t>::value)
        {
            BOOST_TEST_NO_THROW(
                gil::fill_pixels(
                    gil::view(dyn_image_lhs3), value_rhs));
            BOOST_TEST(
                gil::equal_pixels(
                    gil::const_view(dyn_image_lhs1),
                    gil::const_view(dyn_image_lhs3)));
        }
        else
        {
            BOOST_TEST_THROWS(
                gil::fill_pixels(
                    gil::view(dyn_image_lhs3), value_rhs),
                std::bad_cast);
        }
    }

    static void run()
    {
        boost::mp11::mp_for_each
        <
            boost::mp11::mp_pairwise_fold
            <
                fixture::image_types, std::pair
            >
        >(test_fill_pixels{});
    }
};

int main()
{
    test_fill_pixels::run();

    return ::boost::report_errors();
}
