# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

 add_executable(test_serialization test_serialization.cpp)                                                  
 target_link_libraries(test_serialization PRIVATE tincup)                                                               
 add_test(NAME SerializationTest COMMAND test_serialization) 
