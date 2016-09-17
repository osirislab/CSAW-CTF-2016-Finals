#include "string.hpp"

#include <iostream> // std::cout, std::cerr

auto operator<<(std::ostream &os, DbgPrCh c) -> std::ostream & {
  if (c.ch == 0) {
    os << "\\0";
  } else if (c.ch < 0x07 || c.ch >= 0x7F) {
    os << "\\x" << std::hex << int(c.ch) << std::dec;
  } else if (c.ch >= 0x20) {
    os << c.ch;
  } else {
    switch (c.ch) {
      case '\a': {
        os << "\\a";
      } break;
      case '\b': {
        os << "\\b";
      } break;
      case '\f': {
        os << "\\f";
      } break;
      case '\t': {
        os << "\\t";
      } break;
      case '\v': {
        os << "\\v";
      } break;
      case '\n': {
        os << "\\n";
      } break;
      case '\r': {
        os << "\\r";
      } break;
      default: {
        std::cerr << "ICE: unreachable: " << __FILE__ << __LINE__ << "\n";
        std::exit(5);
      } break;
    }
  }

  return os;
}

auto operator<<(std::ostream &os, DbgPrStr s) -> std::ostream & {
  for (size_t i = 0; i < s.s.length (); ++i) {
    os << DbgPrCh({s.s[i]});
  }
  return os;
}

Str::Str(): ptr_(nullptr), len_(0) { }
Str::Str(uint8_t const *ptr, size_t len): ptr_(ptr), len_(len) { }
Str::Str(char const *ptr, size_t len):
  Str(reinterpret_cast<uint8_t const *>(ptr), len) { }

auto Str::start() const noexcept -> uint8_t const * {
  return ptr_;
}
auto Str::data() const noexcept -> uint8_t const * {
  return ptr_;
}
auto Str::length() const noexcept -> size_t {
  return len_;
}

auto operator<<(std::ostream &os, Str const &s) -> std::ostream & {
  for (
    auto &&start = s.start(), end = s.start() + s.length();
    start != end;
    ++start
  ) {
    os << *start;
  }
  return os;
}

auto operator==(Str lhs, Str rhs) noexcept -> bool {
  if (lhs.length() != rhs.length()) {
    return false;
  }
  auto len = lhs.length();
  for (auto i = size_t (0); i < len; ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

auto Str::operator[](size_t idx) const noexcept -> uint8_t const & {
  return ptr_[idx];
}

static auto length_of_char(uint8_t c) -> size_t {
  static const uint8_t lookup_table[128] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4,4,5,5,5,5,6,6
  };

  if (c & (1 << 7)) {
    return lookup_table[c & ~(1 << 7)];
  } else {
    return 1;
  }
}

String::String() noexcept :  sso_ {}, length_(0) { }

String::~String() {
  if (length_ > 16) {
    std::free(fatptr_.ptr);
  }
}

String::String(Str s): String() {
  push(s);
}

String::String(char const *s, size_t len)
  : String(reinterpret_cast<uint8_t const *>(s), len) {}

String::String(uint8_t const *s, size_t len): String(Str(s, len)) { }

String::String(String const &other) {
  if (other.length_ <= 16) {
    std::memcpy(sso_, &other.sso_, other.length_);
  } else {
    if (auto tmp = std::malloc(other.fatptr_.cap)) {
      fatptr_.ptr = reinterpret_cast<uint8_t *>(tmp);
    } else {
      std::exit(-1);
    }
    fatptr_.cap = other.fatptr_.cap;
    std::memcpy(fatptr_.ptr, other.fatptr_.ptr, other.length_);
  }
  length_ = other.length_;
}

String::String(String &&other) noexcept : String() {
  swap(*this, other);
}

auto String::operator=(String other) -> String & {
  swap(*this, other);
  return *this;
}


auto String::data() noexcept -> uint8_t * {
  return (length_ <= 16 ? sso_ : fatptr_.ptr);
}
auto String::data() const noexcept -> uint8_t const * {
  return (length_ <= 16 ? sso_ : fatptr_.ptr);
}
auto String::length() const noexcept -> size_t {
  return length_;
}

String::operator Str() const noexcept {
  return Str(data(), length());
}

auto String::operator[](size_t idx) noexcept -> uint8_t & {
  return data()[idx];
}

auto String::operator[](size_t idx) const noexcept -> uint8_t const & {
  return data()[idx];
}

auto String::push(Str to_push) -> void {
  auto n = size_t(0);
  while (n < to_push.length()) {
    if (length_ < 16) { // sso
      // make sure to check the utf8
      auto length = length_of_char(to_push[n]) - 1;
      sso_[length_++] = to_push[n++];

      if (to_push.length() - n < length) {
        std::cerr << "attempted to push invalid utf8\n";
        std::exit(3);
      }
      while (length && (n < to_push.length())) {
        auto tmp = to_push[n++];
        sso_[length_++] = tmp;
        --length;
      }
    } else if (length_ == 16) { // switch from sso to non-sso
      if (auto ptr = std::malloc(length_ * 2)) {
        std::memcpy (ptr, sso_, length_);
        fatptr_.ptr = reinterpret_cast<uint8_t *>(ptr);
        fatptr_.cap = length_ * 2;
        fatptr_.ptr[length_++] = to_push[n++];
      } else {
        std::exit(-1);
      }
    } else { // non-sso
      if (length_ < fatptr_.cap) {
        fatptr_.ptr[length_++] = to_push[n++];
      } else {
        if (auto ptr = std::realloc(fatptr_.ptr, length_ * 2)) {
          fatptr_.ptr = reinterpret_cast<uint8_t *>(ptr);
          fatptr_.cap = length_ * 2;
          fatptr_.ptr[length_++] = to_push[n++];
        } else {
          std::exit(-1);
        }
      }
    }
  }
}

auto swap(String &lhs, String &rhs) noexcept -> void {
  using std::swap;
  uint8_t tmp_buffer[16] = {};
  if (lhs.length_ <= 16 && rhs.length_ <= 16) {
    swap(lhs.length_, rhs.length_);
    swap(lhs.sso_, rhs.sso_);
  } else if (lhs.length_ <= 16) {
    swap(tmp_buffer, lhs.sso_);
    swap(lhs.length_, rhs.length_);
    lhs.fatptr_ = rhs.fatptr_;
    swap(tmp_buffer, rhs.sso_);
  } else if (rhs.length_ <= 16) {
    swap(tmp_buffer, rhs.sso_);
    swap(rhs.length_, lhs.length_);
    rhs.fatptr_ = lhs.fatptr_;
    swap(tmp_buffer, lhs.sso_);
  } else {
    swap(lhs.length_, rhs.length_);
    swap(lhs.fatptr_, rhs.fatptr_);
  }
}

auto operator<<(std::ostream &os, String const &s) -> std::ostream & {
  os << Str(s);
  return os;
}

namespace npm {

template<>
class from_str<double> {
  Str s;
public:
  auto parse() const -> double;
};

auto from_str<double>::parse() const -> double {
  if (s.length() == 0) {
    std::cerr << "empty string passed to from_str<double>::parse\n";
    std::exit(5);
  } else {
    auto ptr = reinterpret_cast<char *>(std::malloc(s.length() + 1));
    if (!ptr) {
      std::exit(-1);
    }
    std::memcpy(ptr, s.data(), s.length());
    ptr[s.length()] = 0;

    char *end;
    auto ret = std::strtod(ptr, &end);
    if (end != ptr + s.length()) {
      std::cerr << "malformed floating point literal\n";
      std::exit(3);
    }
    return ret;
  }
}

} // namespace npm

