/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * @file type_list.hpp
 * @brief Defines compile time type container and associated template metaprogramming utilities
 */

#pragma once
#include <typeindex>
#include "type_utils.hpp"

namespace tincup {

/**
 * @brief A helper type trait for finding the index of a given type in variadic parameter pack
 */
template<class, class...>
struct IndexOf;


template<class First, class Second, class... Rest>
struct IndexOf<First, Second, Rest...> : std::conditional_t<std::is_same_v<std::remove_cvref_t<First>, std::remove_cvref_t<Second>>,
                                                            size_constant<0>,
                                                            size_constant<1 + IndexOf<First, Rest...>::value>> {};

template<class T>
struct IndexOf<T> : size_constant<0> {};


template<class...> struct TypeList;

namespace detail {
template<class> 
struct IsTypeList : std::false_type {};

template<class...Ts>
struct IsTypeList<TypeList<Ts...>> : std::true_type {};
};

/**
 * @concept type_list_c
 * @brief Concept for types that satisfy the TypeList interface.
 */
template<class TL>
concept type_list_c = detail::IsTypeList<TL>::value;

template<class TL>
concept unique_type_list_c = type_list_c<TL> && TL::is_unique;

template<class TL>
concept nonempty_type_list_c = type_list_c<TL> && (TL::size > 0);
                             


/**
 * @class TypeListElement
 * @brief Retrieves the type at a specific index in a TypeList.
 *
 * @tparam I The index to retrieve.
 * @tparam TL The TypeList to retrieve from.
 */
template<std::size_t, type_list_c>
struct TypeListElement;


/**
 * @class TypeListElement<0u,TypeList<First,Rest...>> 
 * @brief Specialization for retrieving the first element of a TypeList.
 *
 * @tparam First The first type in the list.
 * @tparam Rest The remaining types in the list.
 */
template<class First, class...Rest>
struct TypeListElement<0u,TypeList<First,Rest...>> {
  using type = First;
};


/**
 * @class TypeListElement<I,TypeList<First,Rest...>> 
 * @brief Specialization for retrieving a non-first element of a TypeList.
 *
 * @tparam I The index to retrieve.
 * @tparam First The first type in the list.
 * @tparam Rest The remaining types in the list.
 */
template<std::size_t I, class First, class...Rest>
requires (I > 0 && I <= sizeof...(Rest))
struct TypeListElement<I,TypeList<First,Rest...>> {
  using type = typename TypeListElement<I-1u,TypeList<Rest...>>::type;
};

/**
 * @class TypeListElement<I,TypeList<Ts...>> 
 * @brief Specialization for handling out-of-bounds indices.
 *
 * @tparam I The index to retrieve.
 * @tparam Ts The types in the list.
 */
template<std::size_t I, class...Ts>
requires (I > sizeof...(Ts))
struct TypeListElement<I,TypeList<Ts...>> {
  // This will cause a compile-time error when accessed
  static_assert(I <= sizeof...(Ts), "Index out of bounds in TypeList");
};
// Note: pop_front is available as a member alias on TypeList


/**
 * @brief Concatenates an arbitrary number of TypeLists.
 *
 * @tparam Lists The TypeLists to concatenate.
 */
template<type_list_c... Lists>
struct ConcatenateTypeLists;


/**
 * @brief Base case: single TypeList.
 *
 * @tparam Ts Types in the single TypeList.
 */
template<class... Ts>
struct ConcatenateTypeLists<TypeList<Ts...>> {
  using type = TypeList<Ts...>;
};

/**
 * @brief Recursive case: two or more TypeLists.
 *
 * @tparam Ts Types in the first TypeList.
 * @tparam Us Types in the second TypeList.
 * @tparam Rest Remaining TypeLists.
 */
template<class... Ts, class... Us, type_list_c... Rest>
struct ConcatenateTypeLists<TypeList<Ts...>, TypeList<Us...>, Rest...> {
  using type = typename ConcatenateTypeLists<TypeList<Ts..., Us...>, Rest...>::type;
};

/**
 * @brief Alias template for concatenating multiple TypeLists.
 *
 * @tparam Lists The TypeLists to concatenate.
 */
template<type_list_c... Lists>
using concatenate_type_lists = typename ConcatenateTypeLists<Lists...>::type;


/**
 * @class TypeList
 * @brief A list of types with various utility operations.
 *
 * @tparam Ts The types in the list.
 */
template<class First, class...Rest>
struct TypeList<First,Rest...> {

  /// Number of types in the list
  static constexpr std::size_t size = 1 + sizeof...(Rest);

  using first_type = First;

  /**
   * @brief Appends a type to the end of the list.
   *
   * @tparam T The type to append.
   */
  template<class T>
  using append = TypeList<First,Rest...,T>;


  /**
   * @brief Retrieves the type at a specific index.
   *
   * @tparam I The index to retrieve.
   */
  template<std::size_t I>
  using type = typename TypeListElement<I, TypeList<First,Rest...>>::type;

  template<std::size_t I>
  requires (I<size)
  static auto get_type_index( size_constant<I> ) {
    return std::type_index(typeid(type<I>));
  }


   /**
   * @brief Removes the first type from the list.
   */
  using pop_front = TypeList<Rest...>;

  /**
   * @brief Adds a type to the front of the list.
   *
   * @tparam T The type to add.
   */
  template<class T>
  using push_front = TypeList<T,First,Rest...>;

  /**
   * @brief Determines if the type T is in the TypeList
   *
   * @tparam T The type to search for.
   *
   * @return constexpr bool True if T is one of the types in the TypeList
   */
  template<class T>
  static constexpr bool contains_type = std::disjunction_v<std::is_same<std::remove_cvref_t<T>,std::remove_cvref_t<First>>,std::is_same<std::remove_cvref_t<T>,std::remove_cvref_t<Rest>>...>;


  /**
   * @brief Finds the index of a type in the list if it exists
   *
   * @tparam T The type to search for.
   */
  template<class T>
  requires contains_type<T>
  static constexpr std::size_t index_of = IndexOf<T,First,Rest...>::value;

  /**
   * @brief Helper function to deduce if there are no repeated types.
   */
  template<std::size_t I = 0, std::size_t J = I + 1>
  static constexpr bool is_unique_impl() {
    if constexpr (I == size) {
      return true;
    } else if constexpr (J == size) {
      return is_unique_impl<I + 1, I + 2>();
    } else {
      return !std::is_same_v<type<I>, type<J>> && is_unique_impl<I, J + 1>();
    }
  }

  static constexpr bool is_unique = is_unique_impl();
};


template<>
struct TypeList<> {
  static constexpr std::size_t size = 0;

  template<class T>
  using append = TypeList<T>;

  template<class T>
  using push_front = TypeList<T>;

  template<class>
  static constexpr bool contains_type = false;

  static constexpr bool is_unique = true;
};

/**
 * @brief Specialization of Increment for TypeLists
 * @tparam Ts The types in the TypeList
 * @note Requires that Ts be incrementable
 */
template<class First, class...Rest>
struct Increment<TypeList<First,Rest...>> {
  using type = TypeList<increment_t<First>,increment_t<Rest>...>;
};

template<>
struct Increment<TypeList<>> {};





/**
 * @class IndexedTypeList
 * @brief A utility class for creating type lists indexed by compile-time values.
 * @tparam T A template that takes a std::size_t parameter and produces a type.
 * @details This class provides a way to create type lists where each element
 * is generated by applying the template T to a sequence of indices.
 */
template<template<std::size_t> class T>
struct IndexedTypeList {
  /**
   * @brief Helper function to create a TypeList from an index sequence.
   * @tparam Is Parameter pack of indices.
   * @return A TypeList containing T<Is>... types.
   */
  template<std::size_t...Is> 
  static constexpr auto eval( std::index_sequence<Is...> ) -> TypeList<T<Is>...> {
    return TypeList<T<Is>...>{};
  }

  /**
   * @brief Creates a TypeList with N elements.
   * @tparam N The number of elements in the resulting TypeList.
   * @return A TypeList containing T<0>, T<1>, ..., T<N-1> types.
   */
  template<std::size_t N> 
  static constexpr auto eval( size_constant<N> ) {
    return eval( std::make_index_sequence<N>{} );
  }

  /**
   * @typedef type
   * @brief Alias for the resulting TypeList.
   * @tparam N The number of elements in the TypeList.
   */
  template<std::size_t N>
  using type = decltype( eval(size_constant<N>{}) );
};


/**
 * @typedef indexed_type_list_t
 * @brief Convenience alias for creating an indexed type list.
 * @tparam T A template that takes a std::size_t parameter and produces a type.
 * @tparam N The number of elements in the resulting TypeList.
 * @details This alias provides a shorthand for creating a TypeList with N elements,
 * where each element is generated by applying the template T to indices 0 to N-1.
 */
template<template<std::size_t> class T, std::size_t N>
using indexed_type_list_t = typename IndexedTypeList<T>::template type<N>;

} // namespace tincup







