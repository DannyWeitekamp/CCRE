#ifndef _CRE_HELPERS_H_
#define _CRE_HELPERS_H_

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <random>


#define rand_flt() (double(std::rand()) / RAND_MAX) 

template <std::floating_point T>
std::string flt_to_str(T x){
	std::string flt_str = fmt::format("{:.4f}", x);
	size_t r_nonzero = flt_str.length()-1;
	for(size_t j=flt_str.length()-1; j >= 1; j--){
		if(flt_str[j] != '0'){
			r_nonzero = j;
			break;
		}
	}
	if(flt_str[r_nonzero] == '.'){
		r_nonzero += 1;
	}
	return flt_str.substr(0, r_nonzero+1);
}

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

#endif /* _CRE_HELPERS_H_ */
