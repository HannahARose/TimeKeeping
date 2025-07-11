#include <iostream>
#include <boost/multiprecision/cpp_bin_float.hpp>

int main() {
    // Check the precision of floating-point types
    std::cout << "Size of float: " << sizeof(float) << " bytes, " 
            << std::numeric_limits<float>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of double: " << sizeof(double) << " bytes, " 
            << std::numeric_limits<double>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of long double: " << sizeof(long double) << " bytes, " 
            << std::numeric_limits<long double>::digits10 << " decimal digits" << std::endl;

    // Check the size of cpp_bin_float types
    std::cout << "Size of cpp_bin_float_single: " << sizeof(boost::multiprecision::cpp_bin_float_single) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_single>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_double: " << sizeof(boost::multiprecision::cpp_bin_float_double) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_double>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_double_extended: " << sizeof(boost::multiprecision::cpp_bin_float_double_extended) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_double_extended>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_double_extended: " << sizeof(boost::multiprecision::cpp_bin_float_double_extended) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_double_extended>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_quad: " << sizeof(boost::multiprecision::cpp_bin_float_quad) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_quad>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_oct: " << sizeof(boost::multiprecision::cpp_bin_float_oct) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_oct>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_50: " << sizeof(boost::multiprecision::cpp_bin_float_50) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_50>::digits10 << " decimal digits" << std::endl;

    std::cout << "Size of cpp_bin_float_100: " << sizeof(boost::multiprecision::cpp_bin_float_100) << " bytes, " 
            << std::numeric_limits<boost::multiprecision::cpp_bin_float_100>::digits10 << " decimal digits" << std::endl;

    return 0;
}