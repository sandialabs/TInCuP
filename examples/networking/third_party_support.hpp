/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * Demonstration of third-party type support using cpo_impl specialization
 * 
 * This shows how library users can add serialization support for types they
 * don't control (e.g., from other libraries) without modifying the original
 * type definition or the serialization library.
 */

#pragma once

#include "binary_backend.hpp"
#include <chrono>
#include <complex>

namespace networking::third_party {

// =============================================================================
// Simulate third-party types that we can't modify
// =============================================================================

// Example 1: Simulate a third-party UUID type
namespace external_lib {
    struct UUID {
        uint64_t high;
        uint64_t low;
        
        UUID() = default;
        UUID(uint64_t h, uint64_t l) : high(h), low(l) {}
        
        bool operator==(const UUID& other) const {
            return high == other.high && low == other.low;
        }
    };
    
    // Another third-party type - a fixed-size buffer
    template<std::size_t N>
    struct FixedBuffer {
        std::array<uint8_t, N> data;
        std::size_t used_size;
        
        FixedBuffer() : used_size(0) { data.fill(0); }
        
        bool operator==(const FixedBuffer& other) const {
            return used_size == other.used_size && 
                   std::equal(data.begin(), data.begin() + used_size, 
                             other.data.begin());
        }
    };
}

} // namespace networking::third_party

// =============================================================================
// Third-party type serialization via cpo_impl specialization
// =============================================================================

namespace tincup {

// Specialization for external UUID type
template<>
struct cpo_impl<networking::serialize_ftor, networking::third_party::external_lib::UUID> {
    void operator()(networking::binary_writer& writer, const networking::third_party::external_lib::UUID& uuid) const {
        networking::serialize(writer, uuid.high, uuid.low);
    }
};

template<>
struct cpo_impl<networking::deserialize_ftor, networking::third_party::external_lib::UUID> {
    void operator()(networking::binary_reader& reader, networking::third_party::external_lib::UUID& uuid) const {
        networking::deserialize(reader, uuid.high);
        networking::deserialize(reader, uuid.low);
    }
};

// Specialization for external FixedBuffer template
template<std::size_t N>
struct cpo_impl<networking::serialize_ftor, networking::third_party::external_lib::FixedBuffer<N>> {
    void operator()(networking::binary_writer& writer, 
                   const networking::third_party::external_lib::FixedBuffer<N>& buffer) const {
        // Serialize the used size, then only the used portion of the buffer
        networking::serialize(writer, buffer.used_size);
        if (buffer.used_size > 0) {
            writer.write_bytes(buffer.data.data(), buffer.used_size);
        }
    }
};

template<std::size_t N>
struct cpo_impl<networking::deserialize_ftor, networking::third_party::external_lib::FixedBuffer<N>> {
    void operator()(networking::binary_reader& reader, 
                   networking::third_party::external_lib::FixedBuffer<N>& buffer) const {
        networking::deserialize(reader, buffer.used_size);
        if (buffer.used_size > N) {
            throw std::runtime_error("FixedBuffer: used_size exceeds buffer capacity");
        }
        if (buffer.used_size > 0) {
            reader.read_bytes(buffer.data.data(), buffer.used_size);
        }
        // Clear unused portion
        std::fill(buffer.data.begin() + buffer.used_size, buffer.data.end(), 0);
    }
};

// Standard library types that could benefit from cpo_impl specialization

// std::complex<T> serialization
template<typename T>
requires std::is_arithmetic_v<T>
struct cpo_impl<networking::serialize_ftor, std::complex<T>> {
    void operator()(networking::binary_writer& writer, const std::complex<T>& c) const {
        networking::serialize(writer, c.real(), c.imag());
    }
};

template<typename T>
requires std::is_arithmetic_v<T>
struct cpo_impl<networking::deserialize_ftor, std::complex<T>> {
    void operator()(networking::binary_reader& reader, std::complex<T>& c) const {
        T real, imag;
        networking::deserialize(reader, real);
        networking::deserialize(reader, imag);
        c = std::complex<T>(real, imag);
    }
};

// std::chrono::time_point serialization (serialize as nanoseconds since epoch)
template<typename Clock, typename Duration>
struct cpo_impl<networking::serialize_ftor, std::chrono::time_point<Clock, Duration>> {
    void operator()(networking::binary_writer& writer, 
                   const std::chrono::time_point<Clock, Duration>& tp) const {
        auto ns_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(
            tp.time_since_epoch()).count();
        networking::serialize(writer, ns_since_epoch);
    }
};

template<typename Clock, typename Duration>
struct cpo_impl<networking::deserialize_ftor, std::chrono::time_point<Clock, Duration>> {
    void operator()(networking::binary_reader& reader, 
                   std::chrono::time_point<Clock, Duration>& tp) const {
        int64_t ns_since_epoch;
        networking::deserialize(reader, ns_since_epoch);
        tp = std::chrono::time_point<Clock, Duration>(
            std::chrono::duration_cast<Duration>(
                std::chrono::nanoseconds(ns_since_epoch)));
    }
};

} // namespace tincup

namespace networking::third_party {

// =============================================================================
// Example usage combining user types and third-party types
// =============================================================================

struct GameSession {
    external_lib::UUID session_id;
    std::chrono::system_clock::time_point start_time;
    std::complex<float> server_coordinates;  // Imagine server location in complex plane
    external_lib::FixedBuffer<256> session_data;
    
    bool operator==(const GameSession& other) const {
        return session_id == other.session_id && 
               start_time == other.start_time &&
               server_coordinates == other.server_coordinates &&
               session_data == other.session_data;
    }
};

// This works automatically because all member types have serialization support!
inline void tag_invoke(serialize_ftor, binary_writer& writer, const GameSession& session) {
    serialize(writer, session.session_id);      // Uses cpo_impl specialization
    serialize(writer, session.start_time);      // Uses cpo_impl specialization
    serialize(writer, session.server_coordinates); // Uses cpo_impl specialization
    serialize(writer, session.session_data);    // Uses cpo_impl specialization
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, GameSession& session) {
    deserialize(reader, session.session_id);
    deserialize(reader, session.start_time);
    deserialize(reader, session.server_coordinates);
    deserialize(reader, session.session_data);
}

} // namespace networking::third_party