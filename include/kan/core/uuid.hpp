#ifndef UUID7_HPP
#define UUID7_HPP

/* ---------------- Example usage ----------------
#include "uuid_v7.hpp"
#include <iostream>

int main() {
    UUIDv7 id = UUIDv7::generate();
    std::cout << id.to_string() << "\n";
    std::cout << "version: " << id.version() << ", variant: " << id.variant() << "\n";

    UUIDv7 parsed = UUIDv7::from_string(id.to_string());
    std::cout << std::boolalpha << (id == parsed) << "\n";
}
------------------------------------------------- */

// uuid_v7.hpp - Self-contained UUIDv7 (RFC 9562) implementation in C++17.
//
// Layout (128 bits, big-endian byte order):
//   48 bits  unix_ts_ms   (milliseconds since Unix epoch)
//    4 bits  version      (0111 = 7)
//   12 bits  rand_a       (random)
//    2 bits  variant      (10)
//   62 bits  rand_b       (random)
//
// No external dependencies beyond the standard library.

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

class UUIDv7 {
public:
  using Bytes = std::array<uint8_t, 16>;

  UUIDv7() : bytes_{} {}

  // Construct from raw 16 bytes.
  explicit UUIDv7(const Bytes &bytes) : bytes_(bytes) {}

  // Parse from canonical string form:
  // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
  static UUIDv7 from_string(const std::string &s) {
    UUIDv7 u;
    size_t byte_idx = 0;
    int    hi       = -1;
    for (char c : s) {
      if (c == '-')
        continue;
      int v = hex_val(c);
      if (v < 0)
        throw std::invalid_argument("UUIDv7::from_string: invalid character");
      if (hi < 0) {
        hi = v;
      }
      else {
        if (byte_idx >= 16)
          throw std::invalid_argument("UUIDv7::from_string: too many hex digits");
        u.bytes_[byte_idx++] = static_cast<uint8_t>((hi << 4) | v);
        hi                   = -1;
      }
    }
    if (byte_idx != 16 || hi != -1)
      throw std::invalid_argument("UUIDv7::from_string: wrong length");
    return u;
  }

  // Generate a new, time-ordered UUIDv7 using a cryptographically
  // reasonable PRNG seeded from std::random_device.
  static UUIDv7 generate() { return generate(now_unix_ms(), rng()); }

  // Deterministic generation, useful for testing.
  static UUIDv7 generate(uint64_t unix_ts_ms, std::mt19937_64 &gen) {
    UUIDv7                                  u;
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());
    uint64_t                                rand_bits_a = dist(gen); // we'll take 12 bits from this
    uint64_t                                rand_bits_b = dist(gen); // we'll take 62 bits from this

    auto &b = u.bytes_;

    // 48-bit big-endian timestamp (bytes 0-5)
    b[0] = static_cast<uint8_t>((unix_ts_ms >> 40) & 0xFF);
    b[1] = static_cast<uint8_t>((unix_ts_ms >> 32) & 0xFF);
    b[2] = static_cast<uint8_t>((unix_ts_ms >> 24) & 0xFF);
    b[3] = static_cast<uint8_t>((unix_ts_ms >> 16) & 0xFF);
    b[4] = static_cast<uint8_t>((unix_ts_ms >> 8) & 0xFF);
    b[5] = static_cast<uint8_t>(unix_ts_ms & 0xFF);

    // byte 6: version (0111) in top nibble + top 4 bits of rand_a
    uint16_t rand_a = static_cast<uint16_t>(rand_bits_a & 0x0FFF); // 12 bits
    b[6]            = static_cast<uint8_t>(0x70 | ((rand_a >> 8) & 0x0F));
    // byte 7: low 8 bits of rand_a
    b[7] = static_cast<uint8_t>(rand_a & 0xFF);

    // byte 8: variant (10) in top 2 bits + top 6 bits of rand_b
    uint64_t rand_b = rand_bits_b & 0x3FFFFFFFFFFFFFFULL; // 62 bits
    b[8]            = static_cast<uint8_t>(0x80 | ((rand_b >> 56) & 0x3F));
    b[9]            = static_cast<uint8_t>((rand_b >> 48) & 0xFF);
    b[10]           = static_cast<uint8_t>((rand_b >> 40) & 0xFF);
    b[11]           = static_cast<uint8_t>((rand_b >> 32) & 0xFF);
    b[12]           = static_cast<uint8_t>((rand_b >> 24) & 0xFF);
    b[13]           = static_cast<uint8_t>((rand_b >> 16) & 0xFF);
    b[14]           = static_cast<uint8_t>((rand_b >> 8) & 0xFF);
    b[15]           = static_cast<uint8_t>(rand_b & 0xFF);

    return u;
  }

  // Extract the embedded 48-bit millisecond timestamp.
  uint64_t timestamp_ms() const {
    return (static_cast<uint64_t>(bytes_[0]) << 40) | (static_cast<uint64_t>(bytes_[1]) << 32) | (static_cast<uint64_t>(bytes_[2]) << 24) |
           (static_cast<uint64_t>(bytes_[3]) << 16) | (static_cast<uint64_t>(bytes_[4]) << 8) | (static_cast<uint64_t>(bytes_[5]));
  }

  int version() const { return (bytes_[6] >> 4) & 0x0F; }
  int variant() const { return (bytes_[8] >> 6) & 0x03; }

  const Bytes &bytes() const { return bytes_; }

  std::string to_string() const {
    static const char *hex = "0123456789abcdef";
    std::string        s;
    s.reserve(36);
    for (size_t i = 0; i < 16; ++i) {
      s.push_back(hex[(bytes_[i] >> 4) & 0x0F]);
      s.push_back(hex[bytes_[i] & 0x0F]);
      if (i == 3 || i == 5 || i == 7 || i == 9)
        s.push_back('-');
    }
    return s;
  }

  bool operator==(const UUIDv7 &other) const { return bytes_ == other.bytes_; }
  bool operator!=(const UUIDv7 &other) const { return bytes_ != other.bytes_; }
  // Lexicographic byte comparison == chronological order for UUIDv7.
  bool operator<(const UUIDv7 &other) const { return bytes_ < other.bytes_; }

private:
  Bytes bytes_;

  static uint64_t now_unix_ms() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
  }

  // Shared, lazily-initialized, thread-local RNG seeded from random_device.
  static std::mt19937_64 &rng() {
    thread_local std::mt19937_64 gen{[] {
      std::random_device rd;
      std::seed_seq      seq{rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd()};
      return std::mt19937_64(seq)();
    }()};
    return gen;
  }

  static int hex_val(char c) {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
  }
};

#endif
