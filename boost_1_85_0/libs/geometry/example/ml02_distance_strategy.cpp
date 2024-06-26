// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2022 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Multipolygon DP simplification example from the mailing list discussion
// about the DP algorithm issue:
// http://lists.osgeo.org/pipermail/ggl/2011-September/001533.html

#include <boost/geometry.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
using namespace boost::geometry;

int main()
{
  typedef model::d2::point_xy<double> point_xy;

  point_xy p1(0.0, 0.0);
  point_xy p2(5.0, 0.0);

  // 1) This is direct call to Pythagoras algo
  using strategy1_type = strategy::distance::pythagoras<double>;
  strategy1_type strategy1;
  auto d1 = strategy1.apply(p1, p2);

  // 2) This is what is effectively called by simplify
  using strategy2_type = strategy::distance::comparable::pythagoras<double>;
  strategy2_type strategy2;
  auto d2 = strategy2.apply(p1, p2);

  return 0;
}
