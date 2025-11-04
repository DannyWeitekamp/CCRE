
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include "../include/helpers.h"




namespace cre {

#define NUMSTR_MAX_LEN 32

// Function to replicate Python-style float printing
std::string flt_to_str(double x){
	// Python str(float()) max length is about 24, 32 adds a little to that.
	fmt::basic_memory_buffer<char, NUMSTR_MAX_LEN> buffer;

	// If the float is an integer and 
	if(x == int64_t(x) && x < 1e16){
		fmt::format_to(std::back_inserter(buffer), "{:0.1f}", x);
	}else{
		fmt::format_to(std::back_inserter(buffer), "{:0.16g}", x);
	}
	
	return std::string(buffer.data(), buffer.size());
	
}

std::string int_to_str(int64_t x){
	// Python str(float()) max length is about 24, 32 adds a little to that.
	fmt::basic_memory_buffer<char, NUMSTR_MAX_LEN> buffer;
	fmt::format_to(std::back_inserter(buffer), "{}", x);	
	return std::string(buffer.data(), buffer.size());
}

// ---- DEAD CODE REMOVE ZEROS -------
// return fmt::format("{:0.15g}", x);
	
	// for(size_t j=buffer.size()-1; j >= 1; j--){
	// 	if(data[j] != '0'){
	// 		r_nonzero = j;
	// 		break;
	// 	}
	// }
	// if(data[r_nonzero] == '.'){
	// 	r_nonzero += 1;
	// }
	// return flt_str.substr(0, r_nonzero+1);
	// return std::string(data, r_nonzero+1);

} // NAMESPACE_END(cre)
