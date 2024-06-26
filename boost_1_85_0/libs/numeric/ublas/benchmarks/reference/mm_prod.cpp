//
// Copyright (c) 2018 Stefan Seefeld
// All rights reserved.
//
// This file is part of Boost.uBLAS. It is made available under the
// Boost Software License, Version 1.0.
// (Consult LICENSE or http://www.boost.org/LICENSE_1_0.txt)

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/program_options.hpp>
#include "../init.hpp"
#include "../benchmark.hpp"
#include <complex>
#include <string>

namespace po = boost::program_options;
namespace ublas = boost::numeric::ublas;
namespace boost { namespace numeric { namespace ublas { namespace benchmark {

template <typename T>
class prod : public benchmark
{
public:
  prod(std::string const &name) : benchmark(name) {}
  virtual void setup(long l)
  {
    init(a, l, 200);
    init(b, l, 200);
  }
  virtual void operation(long l)
  {
    for (int i = 0; i < l; ++i)
      for (int j = 0; j < l; ++j)
      {
	c(i,j) = 0;
	for (int k = 0; k < l; ++k)
	  c(i,j) += a(i,k) * b(k,j);
      }
  }
private:
  ublas::matrix<T> a;
  ublas::matrix<T> b;
  ublas::matrix<T> c;
};

}}}}

namespace bm = boost::numeric::ublas::benchmark;

template <typename T>
void benchmark(std::string const &type)
{
  //  using matrix = ublas::matrix<T, ublas::basic_row_major<>>;
  bm::prod<T> p("ref::prod(matrix<" + type + ">)");
  p.run(std::vector<long>({1, 2, 4, 8, 16, 32, 64, 128, 256, 512}));//, 1024}));
}

int main(int argc, char **argv)
{
  po::variables_map vm;
  try
  {
    po::options_description desc("Matrix product (reference implementation)\n"
                                 "Allowed options");
    desc.add_options()("help,h", "produce help message");
    desc.add_options()("type,t", po::value<std::string>(), "select value-type (float, double, fcomplex, dcomplex)");

    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return 0;
    }
  }
  catch(std::exception &e)
  {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
  std::string type = vm.count("type") ? vm["type"].as<std::string>() : "float";
  if (type == "float")
    benchmark<float>("float");
  else if (type == "double")
    benchmark<double>("double");
  else if (type == "fcomplex")
    benchmark<std::complex<float>>("std::complex<float>");
  else if (type == "dcomplex")
    benchmark<std::complex<double>>("std::complex<double>");
  else
    std::cerr << "unsupported value-type \"" << vm["type"].as<std::string>() << '\"' << std::endl;
}
