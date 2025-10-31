#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <random>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <iterator>


namespace cre {

#define rand_flt() (double(std::rand()) / RAND_MAX) 


std::string flt_to_str(double x);
std::string int_to_str(int64_t x);


template <std::floating_point T>
std::ostream& operator<<(std::ostream& out, std::vector<T> vec){
	std::vector<std::string> float_strs = {};
	float_strs.reserve(vec.size());
	for (size_t i = 0; i < vec.size(); ++i) {
		float_strs.push_back(flt_to_str(vec[i]));
	}
	return out << fmt::format("[{}]", fmt::join(float_strs, ", "));
}

template <std::integral T>
std::ostream& operator<<(std::ostream& out, std::vector<T> vec){
	return out << fmt::format("{}",vec);
}

} // NAMESPACE_END(cre)

