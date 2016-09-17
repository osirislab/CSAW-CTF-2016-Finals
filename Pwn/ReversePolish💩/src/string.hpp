#ifndef STRING_HPP
#define STRING_HPP

#include "types.hpp"

#include <functional> // std::hash
#include <stdexcept> // std::runtime_error
#include <cstdlib> // std::malloc, std::strtod
#include <cstring> // std::memcpy

class Str {
  uint8_t const *ptr_;
  size_t len_;
public:
  Str();
  explicit Str(uint8_t const *ptr, size_t len);
  explicit Str(char const *ptr, size_t len);

  template <typename T>
  auto parse() const -> T;

  template <size_t N>
  constexpr Str(char const (&)[N]);

  auto start() const noexcept -> uint8_t const *;
  auto data() const noexcept -> uint8_t const *;
  auto length() const noexcept -> size_t;

  auto operator[](size_t) const noexcept -> uint8_t const &;
};
template <size_t N>
inline constexpr
Str::Str(char const (&literal)[N]): Str(literal, N - 1) { }

auto operator<<(std::ostream &os, Str const &s) -> std::ostream &;
auto operator==(Str lhs, Str rhs) noexcept -> bool;

namespace npm {

template <typename T>
class from_str {
  Str s;
public:
  from_str(Str s): s(s) {}
  auto parse() const -> T;
};

} // namespace npm
template <typename T>
auto Str::parse() const -> T {
  return npm::from_str<T>(*this).parse();
}

struct utf8_error: public std::runtime_error {
  explicit utf8_error(const char *what_arg): runtime_error(what_arg) { }
};

struct DbgPrCh {
  uint8_t ch;
};
auto operator<<(std::ostream &os, DbgPrCh c) -> std::ostream &;

struct DbgPrStr {
  Str s;
};
auto operator<<(std::ostream &os, DbgPrStr s) -> std::ostream &;

class String {
  union {
    struct {
      uint8_t *ptr;
      size_t cap;
    } fatptr_;
    uint8_t sso_[16];
  };
  size_t length_;

public:
  String() noexcept;
  ~String();

  explicit String(Str s);
  explicit String(char const *s, size_t len);
  explicit String(uint8_t const *s, size_t len);
  String(String const &other);
  String(String &&other) noexcept;
  auto operator=(String other) -> String&;

  friend auto swap(String &lhs, String &rhs) noexcept -> void;

  virtual auto data() noexcept -> uint8_t *;
  virtual auto data() const noexcept -> uint8_t const *;
  virtual auto length() const noexcept -> size_t;

  operator Str() const noexcept;
  virtual auto operator[](size_t) noexcept -> uint8_t &;
  virtual auto operator[](size_t) const noexcept -> uint8_t const &;

  virtual auto push(Str str) -> void;
};

namespace std {

template <>
struct hash<::Str> {
public:
  auto operator()(Str const &s) const -> size_t {
    auto hash = size_t(5381);
    for (auto i = size_t(0); i < s.length(); ++i) {
      hash = ((hash << 5) + hash) + s[i]; /* hash * 33 + s[i] */
    }

    return hash;
  }
};

template <>
struct hash<::String> {
public:
  auto operator()(String const &s) const -> size_t {
    return hash<::Str>()(s);
  }
};

} // namespace std

#endif // STRING_HPP
