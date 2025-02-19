
//  (C) Copyright Nick Thompson 2020.
//  (C) Copyright John Maddock 2020.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#include <iostream>
#include <boost/math/tools/ulps_plot.hpp>
#include <boost/core/demangle.hpp>
#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/cpp_bin_float.hpp>

using boost::math::tools::ulps_plot;

int main() {
   using PreciseReal = boost::multiprecision::mpfr_float_100;
   using CoarseReal = boost::multiprecision::cpp_bin_float_50;

   typedef boost::math::policies::policy<
      boost::math::policies::promote_float<false>,
      boost::math::policies::promote_double<false> >
      no_promote_policy;

   auto ai_coarse = [](CoarseReal const& x)->CoarseReal {
      return atan(x);
   };
   auto ai_precise = [](PreciseReal const& x)->PreciseReal {
      return atan(x);
   };

   std::string filename = "cpp_bin_float_atan.svg";
   int samples = 100000;
   // How many pixels wide do you want your .svg?
   int width = 700;
   // Near a root, we have unbounded relative error. So for functions with roots, we define an ULP clip:
   PreciseReal clip = 50;
   // Should we perturb the abscissas?
   bool perturb_abscissas = false;
   auto plot = ulps_plot<decltype(ai_precise), PreciseReal, CoarseReal>(ai_precise, CoarseReal(-200), CoarseReal(200), samples, perturb_abscissas);
   // Note the argument chaining:
   plot/*.clip(clip)*/.width(width);
   plot.background_color("white").font_color("black");
   // Sometimes it's useful to set a title, but in many cases it's more useful to just use a caption.
   std::string title = "atan ULP plot with cpp_bin_float_50";
   plot.title(title);
   plot.vertical_lines(6);
   plot.add_fn(ai_coarse);
   // You can write the plot to a stream:
   //std::cout << plot;
   // Or to a file:
   plot.write(filename);
}
