/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <iostream>
#include <string>
#include "serialize.hpp"

int main() {
    // Test if the CPO can find the int overload
    int value = 42;
    auto result = tincup::serial::serialize(value);
    std::cout << "Int serialization: " << result << std::endl;
    
    // Test string
    std::string str = "test";
    auto result2 = tincup::serial::serialize(str);
    std::cout << "String serialization: " << result2 << std::endl;
    
    return 0;
}