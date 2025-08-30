/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once
#include <tincup/tincup.hpp>
#include <string>
#include <vector>

// Define the json_string helpers inline in the header
namespace json_string {
  inline std::string object(const std::string& content) {
    return "{" + content + "}";
  }

  inline std::string key_value(const std::string& key, const std::string& value) {
    return "\"" + key + "\": " + value;
  }

  inline std::string quote(const std::string& s) {
    return "\"" + s + "\"";
  }

  inline std::string array(const std::vector<std::string>& items) {
    std::string result = "[";
    for (size_t i = 0; i < items.size(); ++i) {
      result += items[i];
      if (i < items.size() - 1) {
        result += ", ";
      }
    }
    result += "]";
    return result;
  }
}

namespace tincup::serial {

/**
 * @brief A Customization Point Object for serializing objects.
 * @cpo
 * @ingroup tincup_cpos
 *
 * This CPO provides a generic, non-intrusive way to serialize any C++ type.
 * It uses the `tag_invoke` pattern, which offers significant advantages over
 * traditional Object-Oriented (OOP) inheritance-based approaches.
 *
 * ### Comparison: CPO (`tag_invoke`) vs. OOP (Inheritance) for Serialization
 *
 * | Feature | OOP Approach (Inheritance) | CPO Approach (`tag_invoke`) |
 * | :--- | :--- | :--- |
 * | **Intrusiveness** | **Highly Intrusive.**<br>You *must* modify a class's definition to make it inherit from a base class. | **Non-Intrusive.**<br>You can make any class serializable without touching its source code by providing a `tag_invoke` overload. |
 * | **Coupling** | **Tight Coupling.**<br>Your data types become directly dependent on the serialization library's base class. | **Decoupled.**<br>Your data types have **zero knowledge** of the serialization system, and vice-versa. |
 * | **Applicability** | **Limited to types you can modify.**<br>Fails for primitives (`int`), `std` types (`vector`), and third-party library types. | **Universal.**<br>Works for **any type**. You can write overloads for primitives, standard library types, and third-party types. |
 * | **Polymorphism** | **Runtime Polymorphism.**<br>Relies on `virtual` functions and v-table lookups, often requiring heap allocation. | **Compile-Time Polymorphism.**<br>Relies on template resolution and Argument-Dependent Lookup (ADL), which is generally faster. |
 * | **Conceptual Model** | **"is-a" relationship.**<br>`MyData` *is-a* `Serializable`, which can be an unnatural modeling constraint. | **"can-be" relationship.**<br>`MyData` *can-be* serialized, treating it as an independent operation. |
 *
 * @cpo_example
 * @code
 * // See examples/serialize/serialize.cpp for a full working example.
 *
 * // In user code, for a user-defined type:
 * namespace my_app {
 *   struct Person { std::string name; int age; };
 *
 *   // Provide the overload in your type's namespace
 *   std::string tag_invoke(tincup::serial::serialize_ftor, const Person& p) {
 *     return "{\"name\": \"" + p.name + "\", \"age\": " + std::to_string(p.age) + "}";
 *   }
 * }
 *
 * // Later...
 * my_app::Person p = {"John", 30};
 * std::string json = tincup::serial::serialize(p);
 * @endcode
 */

inline constexpr struct serialize_ftor final : tincup::cpo_base<serialize_ftor> {
  TINCUP_CPO_TAG("serialize")
    // Typed operator() overload - positive case (generic)
  template<typename T>
    requires tincup::invocable_c<serialize_ftor, const T&>
  constexpr auto operator()(const T& obj) const
    noexcept(tincup::nothrow_invocable_c<serialize_ftor, const T&>)
    -> tincup::invocable_t<serialize_ftor, const T&> {
    return tag_invoke(*this, obj);
  }

} serialize;

// Usage: tincup::is_invocable_v<serialize_ftor, const T&>
// Usage: tincup::invocable_t<serialize_ftor, const T&>
// Usage: serialize_ftor::result_t<const T&> (clean return type alias)
// Usage: tincup::cpo_traits<serialize_ftor, const T&>

// Template overload for fundamental arithmetic types
template<typename T>
  requires std::is_arithmetic_v<T>
std::string tag_invoke(serialize_ftor, T value) {
  return std::to_string(value);
}

} // namespace tincup::serial

// std namespace overloads for ADL
namespace std {
  inline std::string tag_invoke(tincup::serial::serialize_ftor, const std::string& value) {
    return json_string::quote(value);
  }

  template<typename T>
  std::string tag_invoke(tincup::serial::serialize_ftor, const std::vector<T>& vec) {
    std::vector<std::string> items;
    for (const auto& item : vec) {
      items.push_back(tincup::serial::serialize(item));
    }
    return json_string::array(items);
  }
}
