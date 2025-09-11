/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * TInCuP Networking Serialization Example
 * 
 * This example demonstrates a flexible serialization system using TInCuP CPOs
 * that supports multiple backends (binary, JSON, etc.) and allows users to
 * extend serialization for their own types.
 */

#pragma once

#include "../../single_include/tincup.hpp"
#include <vector>
#include <string>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace networking {

// Forward declarations
class binary_writer;
class binary_reader;
class json_writer;
class json_reader;

// Concepts for serialization backends
template<typename Writer>
concept binary_writer_c = requires(Writer& w, const void* data, std::size_t size) {
    w.write_bytes(data, size);
};

template<typename Reader>
concept binary_reader_c = requires(Reader& r, void* data, std::size_t size) {
    r.read_bytes(data, size);
};

template<typename Writer>
concept json_writer_c = requires(Writer& w, std::string_view key, std::string_view value) {
    w.write_key_value(key, value);
    w.begin_object();
    w.end_object();
    w.begin_array();
    w.end_array();
};

// =============================================================================
// Serialize CPO - The main customization point for serialization
// =============================================================================

struct serialize_ftor;

inline constexpr struct serialize_ftor final : tincup::cpo_base<serialize_ftor> {
    TINCUP_CPO_TAG("serialize")
    using tincup::cpo_base<serialize_ftor>::operator();
    inline static constexpr bool is_variadic = false;
    
    // Primary interface: serialize(writer, value)
    template<typename Writer, typename T>
    requires tincup::invocable_c<serialize_ftor, Writer&, T>
    constexpr auto operator()(Writer& writer, T&& value) const
    noexcept(tincup::nothrow_invocable_c<serialize_ftor, Writer&, T>)
    -> tincup::invocable_t<serialize_ftor, Writer&, T> {
        return tag_invoke(*this, writer, std::forward<T>(value));
    }
    
    // Variadic interface: serialize(writer, value1, value2, ...)
    template<typename Writer, typename... Args>
    requires (sizeof...(Args) > 1) && (tincup::invocable_c<serialize_ftor, Writer&, Args> && ...)
    constexpr void operator()(Writer& writer, Args&&... values) const
    noexcept((tincup::nothrow_invocable_c<serialize_ftor, Writer&, Args> && ...)) {
        (tag_invoke(*this, writer, std::forward<Args>(values)), ...);
    }
} serialize;

// =============================================================================
// Deserialize CPO - The main customization point for deserialization  
// =============================================================================

struct deserialize_ftor;

inline constexpr struct deserialize_ftor final : tincup::cpo_base<deserialize_ftor> {
    TINCUP_CPO_TAG("deserialize")
    using tincup::cpo_base<deserialize_ftor>::operator();
    inline static constexpr bool is_variadic = false;
    
    // Primary interface: deserialize(reader, value)
    template<typename Reader, typename T>
    requires tincup::invocable_c<deserialize_ftor, Reader&, T&>
    constexpr auto operator()(Reader& reader, T& value) const
    noexcept(tincup::nothrow_invocable_c<deserialize_ftor, Reader&, T&>)
    -> tincup::invocable_t<deserialize_ftor, Reader&, T&> {
        return tag_invoke(*this, reader, value);
    }
    
    // Factory interface: auto value = deserialize<T>(reader)
    template<typename T, typename Reader>
    requires tincup::invocable_c<deserialize_ftor, Reader&, T&>
    constexpr T operator()(Reader& reader) const
    noexcept(tincup::nothrow_invocable_c<deserialize_ftor, Reader&, T&>) {
        T value;
        tag_invoke(*this, reader, value);
        return value;
    }
} deserialize;

// =============================================================================
// Built-in serialization for fundamental types
// =============================================================================

// Fundamental types - binary serialization
template<typename T>
requires std::is_fundamental_v<T>
void tag_invoke(serialize_ftor, binary_writer& writer, const T& value);

template<typename T>  
requires std::is_fundamental_v<T>
void tag_invoke(deserialize_ftor, binary_reader& reader, T& value);

// String serialization
void tag_invoke(serialize_ftor, binary_writer& writer, const std::string& str);
void tag_invoke(deserialize_ftor, binary_reader& reader, std::string& str);

// Vector serialization (for any serializable element type)
template<typename T>
requires tincup::invocable_c<serialize_ftor, binary_writer&, const T&>
void tag_invoke(serialize_ftor, binary_writer& writer, const std::vector<T>& vec);

template<typename T>
requires tincup::invocable_c<deserialize_ftor, binary_reader&, T&>
void tag_invoke(deserialize_ftor, binary_reader& reader, std::vector<T>& vec);

// JSON serialization for fundamental types
template<typename T>
requires std::is_arithmetic_v<T>
void tag_invoke(serialize_ftor, json_writer& writer, const T& value);

void tag_invoke(serialize_ftor, json_writer& writer, const std::string& str);
void tag_invoke(serialize_ftor, json_writer& writer, bool value);

} // namespace networking