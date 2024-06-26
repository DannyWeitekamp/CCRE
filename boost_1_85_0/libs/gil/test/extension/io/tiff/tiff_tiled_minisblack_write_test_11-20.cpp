//
// Copyright 2013 Christian Henning
//
// Distributed under the Boost Software License, Version 1.0
// See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt
//
#include <boost/gil.hpp>
#include <boost/gil/extension/io/tiff.hpp>

#include <boost/core/lightweight_test.hpp>

#include <string>

#include "tiff_tiled_write_macros.hpp"

namespace gil = boost::gil;

#ifdef BOOST_GIL_IO_USE_TIFF_GRAPHICSMAGICK_TEST_SUITE_IMAGES

BOOST_PP_REPEAT_FROM_TO(11, 16, GENERATE_WRITE_TILE_BIT_ALIGNED_MINISBLACK, minisblack)
BOOST_PP_REPEAT_FROM_TO(17, 21, GENERATE_WRITE_TILE_BIT_ALIGNED_MINISBLACK, minisblack)

void test_write_minisblack_tile_and_compare_with_16()
{
    std::string filename_strip(tiff_in_GM + "tiger-minisblack-strip-16.tif");

    gil::gray16_image_t img_strip, img_saved;
    gil::read_image(filename_strip, img_strip, gil::tiff_tag());

    gil::image_write_info<gil::tiff_tag> info;
    info._is_tiled   = true;
    info._tile_width = info._tile_length = 16;

#ifdef BOOST_GIL_IO_TEST_ALLOW_WRITING_IMAGES
    gil::write_view(tiff_out + "write_minisblack_tile_and_compare_with_16.tif", gil::view(img_strip), info);
    gil::read_image(tiff_out + "write_minisblack_tile_and_compare_with_16.tif", img_saved, gil::tiff_tag());

    BOOST_TEST_EQ(gil::equal_pixels(gil::const_view(img_strip), gil::const_view(img_saved)), true);
#endif  // BOOST_GIL_IO_TEST_ALLOW_WRITING_IMAGES
}

int main()
{
    test_write_minisblack_tile_and_compare_with_16();

    // TODO: Make sure generated test cases are executed. See tiff_subimage_test.cpp. ~mloskot

    return boost::report_errors();
}

#else
int main() {}
#endif // BOOST_GIL_IO_USE_TIFF_GRAPHICSMAGICK_TEST_SUITE_IMAGES
