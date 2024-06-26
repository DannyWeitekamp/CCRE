// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2009-2021 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_BOOST_TEST

#ifndef BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE
#define BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE
#endif

#include <test_overlay_p_q.hpp>
#include <common/make_random_generator.hpp>

#include <sstream>
#include <chrono>

#include <boost/program_options.hpp>


template <typename Polygon, typename Generator>
inline void make_polygon(Polygon& polygon, Generator& generator, bool triangular)
{
    typedef typename bg::point_type<Polygon>::type point_type;
    typedef typename bg::coordinate_type<Polygon>::type coordinate_type;

    coordinate_type x, y;
    x = generator();
    y = generator();

    typename bg::ring_type<Polygon>::type& ring = bg::exterior_ring(polygon);

    point_type p;
    bg::set<0>(p, x); bg::set<1>(p, y); ring.push_back(p);
    bg::set<0>(p, x); bg::set<1>(p, y + 1); ring.push_back(p);
    bg::set<0>(p, x + 1); bg::set<1>(p, y + 1); ring.push_back(p);
    bg::set<0>(p, x + 1); bg::set<1>(p, y); ring.push_back(p);
    bg::set<0>(p, x); bg::set<1>(p, y); ring.push_back(p);

    if (triangular)
    {
        // Remove a point depending on generator
        int c = generator() % 4;
        if (c >= 1 && c <= 3)
        {
            ring.erase(ring.begin() + c);
        }
    }
}



template <typename MultiPolygon, typename Generator>
bool test_recursive_boxes(MultiPolygon& result, int& index,
            Generator& generator,
            int level, bool triangular, p_q_settings const& settings)
{
    MultiPolygon p, q;

    // Generate two boxes
    if (level == 0)
    {
        p.resize(1);
        q.resize(1);
        make_polygon(p.front(), generator, triangular);
        make_polygon(q.front(), generator, triangular);
        bg::correct(p);
        bg::correct(q);
    }
    else
    {
        bg::correct(p);
        bg::correct(q);
        if (! test_recursive_boxes(p, index, generator, level - 1, triangular, settings)
            || ! test_recursive_boxes(q, index, generator, level - 1, triangular, settings))
        {
            return false;
        }
    }

    typedef typename boost::range_value<MultiPolygon>::type polygon;

    std::ostringstream out;
    out << "recursive_box_" << index++ << "_" << level;

    if (! test_overlay_p_q
        <
            polygon,
            typename bg::coordinate_type<MultiPolygon>::type
        >(out.str(), p, q, settings))
    {
        return false;
    }

    MultiPolygon mp;
    bg::detail::union_::union_insert
        <
            polygon
        >(p, q, std::back_inserter(mp));

    bg::unique(mp);
    bg::simplify(mp, result, 0.01);
    bg::correct(mp);
    return true;
}


template <typename T, bool Clockwise, bool Closed>
void test_all(int seed, int count, int field_size, int level, bool triangular, p_q_settings const& settings)
{
    auto const t0 = std::chrono::high_resolution_clock::now();

    auto coordinate_generator = make_int_generator(seed, field_size);

    typedef bg::model::polygon
        <
            bg::model::d2::point_xy<T>, Clockwise, Closed
        > polygon;
    typedef bg::model::multi_polygon<polygon> mp;


    int index = 0;
    int errors = 0;
    for(int i = 0; i < count; i++)
    {
        mp p;
        if (! test_recursive_boxes<mp>(p, index, coordinate_generator, level, triangular, settings))
        {
            errors++;
        }
    }

    auto const t = std::chrono::high_resolution_clock::now();
    auto const elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count();
    std::cout << std::endl
        << "geometries: " << index
        << " errors: " << errors
        << " type: " << string_from_type<T>::name()
        << " time: " << elapsed_ms / 1000.0 << std::endl;
}

int main(int argc, char** argv)
{
    BoostGeometryWriteTestConfiguration();
    try
    {
        namespace po = boost::program_options;
        po::options_description description("=== recursive_polygons ===\nAllowed options");

        int count = 1;
        int seed = -1;
        int level = 3;
        int field_size = 10;
        bool ccw = false;
        bool open = false;
        p_q_settings settings;
        std::string form = "box";

        description.add_options()
            ("help", "Help message")
            ("seed", po::value<int>(&seed), "Initialization seed for random generator")
            ("count", po::value<int>(&count), "Number of tests")
            ("diff", po::value<bool>(&settings.also_difference), "Include testing on difference")
            ("validity", po::value<bool>(&settings.validity)->default_value(true), "Include testing on validity")
            ("level", po::value<int>(&level), "Level to reach (higher->slower)")
            ("size", po::value<int>(&field_size), "Size of the field")
            ("form", po::value<std::string>(&form)->default_value("box"), "Form of the polygons (box, triangle)")
#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
            ("ccw", po::value<bool>(&ccw), "Counter clockwise polygons")
            ("open", po::value<bool>(&open), "Open polygons")
#endif
            ("verbose", po::value<bool>(&settings.verbose), "Verbose")
            ("wkt", po::value<bool>(&settings.wkt), "Create a WKT of the inputs, for all tests")
            ("svg", po::value<bool>(&settings.svg), "Create a SVG for all tests")
        ;

        po::variables_map varmap;
        po::store(po::parse_command_line(argc, argv, description), varmap);
        po::notify(varmap);

        if (varmap.count("help")
            || (form != "box" && form != "triangle"))
        {
            std::cout << description << std::endl;
            return 1;
        }

        bool triangular = form != "box";

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
        if (ccw && open)
        {
            test_all<default_test_type, false, false>(seed, count, field_size, level, triangular, settings);
        }
        else if (ccw)
        {
            test_all<default_test_type, false, true>(seed, count, field_size, level, triangular, settings);
        }
        else if (open)
        {
            test_all<default_test_type, true, false>(seed, count, field_size, level, triangular, settings);
        }
        else
#endif
        {
            test_all<default_test_type, true, true>(seed, count, field_size, level, triangular, settings);
        }
    }
    catch(std::exception const& e)
    {
        std::cout << "Exception " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cout << "Other exception" << std::endl;
    }

    return 0;
}
