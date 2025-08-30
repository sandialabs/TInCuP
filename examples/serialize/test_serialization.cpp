/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "serialize.hpp"

// The json_string helpers are now defined inline in serialize.hpp

// Overloads for fundamental types are now in serialize.hpp

// Define our own application-specific type
namespace my_app {
  struct Person {
    std::string name;
    int age;
  };

  // Provide a tag_invoke overload for our own type in our own namespace
  std::string tag_invoke(tincup::serial::serialize_ftor, const Person& p) {
    return json_string::object(
      json_string::key_value("name", tincup::serial::serialize(p.name)) + ", " +
      json_string::key_value("age", tincup::serial::serialize(p.age))
    );
  }
}

int main() {
  // Use the CPO on our custom type
  my_app::Person person{"John Doe", 30};
  const std::string person_json = tincup::serial::serialize(person);
  const std::string expected_person_json = "{\"name\": \"John Doe\", \"age\": 30}";

  std::cout << "Serialized Person: " << person_json << std::endl;
  assert(person_json == expected_person_json);

  // Use the CPO on a foreign type (std::vector)
  std::vector<int> numbers = {1, 2, 3, 4, 5};
  const std::string numbers_json = tincup::serial::serialize(numbers);
  const std::string expected_numbers_json = "[1, 2, 3, 4, 5]";

  std::cout << "Serialized Vector: " << numbers_json << std::endl;
  assert(numbers_json == expected_numbers_json);
  
  std::cout << "Serialization tests passed!" << std::endl;

  return 0;
}
