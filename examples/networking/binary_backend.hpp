/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * Binary serialization backend implementation
 */

#pragma once

#include "serialize.hpp"
#include <vector>
#include <cstring>
#include <stdexcept>

namespace networking {

// =============================================================================
// Binary Writer - Writes data to a byte buffer
// =============================================================================

class binary_writer {
private:
    std::vector<uint8_t> buffer_;
    
public:
    binary_writer() = default;
    
    // Write raw bytes to the buffer
    void write_bytes(const void* data, std::size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        buffer_.insert(buffer_.end(), bytes, bytes + size);
    }
    
    // Get the serialized data
    const std::vector<uint8_t>& data() const { return buffer_; }
    std::vector<uint8_t> release_data() { return std::move(buffer_); }
    
    // Get size of serialized data
    std::size_t size() const { return buffer_.size(); }
    
    // Clear the buffer
    void clear() { buffer_.clear(); }
};

// =============================================================================
// Binary Reader - Reads data from a byte buffer
// =============================================================================

class binary_reader {
private:
    const uint8_t* data_;
    std::size_t size_;
    std::size_t position_;
    
public:
    binary_reader(const std::vector<uint8_t>& buffer) 
        : data_(buffer.data()), size_(buffer.size()), position_(0) {}
        
    binary_reader(const uint8_t* data, std::size_t size)
        : data_(data), size_(size), position_(0) {}
    
    // Read raw bytes from the buffer
    void read_bytes(void* dest, std::size_t size) {
        if (position_ + size > size_) {
            throw std::runtime_error("binary_reader: Insufficient data to read");
        }
        std::memcpy(dest, data_ + position_, size);
        position_ += size;
    }
    
    // Check if we can read more data
    bool has_data(std::size_t size = 1) const {
        return position_ + size <= size_;
    }
    
    // Get current position
    std::size_t position() const { return position_; }
    
    // Get remaining bytes
    std::size_t remaining() const { return size_ - position_; }
};

// =============================================================================
// Binary serialization implementations for fundamental types
// =============================================================================

// Fundamental types
template<typename T>
requires std::is_fundamental_v<T>
void tag_invoke(serialize_ftor, binary_writer& writer, const T& value) {
    writer.write_bytes(&value, sizeof(T));
}

template<typename T>
requires std::is_fundamental_v<T>  
void tag_invoke(deserialize_ftor, binary_reader& reader, T& value) {
    reader.read_bytes(&value, sizeof(T));
}

// Enum serialization (as underlying type)
template<typename T>
requires std::is_enum_v<T>
void tag_invoke(serialize_ftor, binary_writer& writer, const T& value) {
    serialize(writer, static_cast<std::underlying_type_t<T>>(value));
}

template<typename T>
requires std::is_enum_v<T>
void tag_invoke(deserialize_ftor, binary_reader& reader, T& value) {
    std::underlying_type_t<T> underlying_value;
    deserialize(reader, underlying_value);
    value = static_cast<T>(underlying_value);
}

// String serialization (length-prefixed)
inline void tag_invoke(serialize_ftor, binary_writer& writer, const std::string& str) {
    std::size_t length = str.size();
    serialize(writer, length);  // Write length first
    if (length > 0) {
        writer.write_bytes(str.data(), length);  // Write string data
    }
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, std::string& str) {
    std::size_t length;
    deserialize(reader, length);  // Read length first
    str.resize(length);
    if (length > 0) {
        reader.read_bytes(str.data(), length);  // Read string data
    }
}

// Vector serialization (length-prefixed, then elements)
template<typename T>
requires tincup::invocable_c<serialize_ftor, binary_writer&, const T&>
void tag_invoke(serialize_ftor, binary_writer& writer, const std::vector<T>& vec) {
    std::size_t size = vec.size();
    serialize(writer, size);  // Write size first
    for (const auto& element : vec) {
        serialize(writer, element);  // Write each element
    }
}

template<typename T>
requires tincup::invocable_c<deserialize_ftor, binary_reader&, T&>
void tag_invoke(deserialize_ftor, binary_reader& reader, std::vector<T>& vec) {
    std::size_t size;
    deserialize(reader, size);  // Read size first
    vec.resize(size);
    for (auto& element : vec) {
        deserialize(reader, element);  // Read each element
    }
}

} // namespace networking