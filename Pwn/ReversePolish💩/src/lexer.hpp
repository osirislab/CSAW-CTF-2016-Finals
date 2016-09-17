#ifndef LEXER_HPP
#define LEXER_HPP

#include "types.hpp"

#include "string.hpp"

struct Location {
  uint32_t line;
  uint32_t column;

  auto newline() -> void;
  auto nextch() -> void;
};

auto operator<<(std::ostream &os, Location const &loc) -> std::ostream &;

class Token {
public:
  enum class Tag {
    Number,
    Ident,

    String,

    OpenBlock,
    CloseBlock,

    None,
    Eof
  };
private:
  Tag tag_;
  String data_;
  Location loc_;

public:
  Token() noexcept;
  explicit Token(Tag, String, Location) noexcept;
  auto tag() const noexcept -> Tag;
  auto data() const noexcept -> Str;
  auto loc() const noexcept -> Location;

  auto is_eof() const noexcept -> bool;
  auto is_none() const noexcept -> bool;
};

auto operator<<(std::ostream &os, Token const &tok) -> std::ostream &;

class Lexer {
  std::istream &buffer_;
  Location current_loc_;

  Token current_tok_;

  auto peek_char() -> int;
  auto get_char() -> int;

public:
  explicit Lexer(std::istream &is);
  auto run() -> void;

  auto peek_tok() -> Token const &;
  auto get_tok() -> Token;

  static auto is_special_char(uint8_t c) -> bool;
};

#endif // LEXER_HPP
