#include <iostream>
#include <boost/multiprecision/cpp_bin_float.hpp>

int main() {

    int size = sizeof(boost::multiprecision::cpp_bin_float_100);
    std::cout << "Size of cpp_bin_float_100: " << size << " bytes" << std::endl;


    return 0;
}