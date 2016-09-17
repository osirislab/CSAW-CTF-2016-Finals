#include "lexer.hpp"

#include <iostream> // std::cout
#include <cctype> // std::isspace, std::isdigit
#include <sstream> // std::stringstream
#include <vector> // std::vector

auto Location::newline() -> void {
  ++line;
  column = 1;
}

auto Location::nextch() -> void {
  ++column;
}

auto operator<<(std::ostream &os, Location const &loc) -> std::ostream & {
  os << "(" << loc.line << ", " << loc.column << ")";
  return os;
}

Token::Token() noexcept: tag_(Tag::None), loc_ {1, 1} { }

Token::Token(Tag tag, String data, Location loc) noexcept
  : tag_(tag), data_(std::move(data)), loc_(loc) { }

auto Token::tag() const noexcept -> Tag {
  return tag_;
}

auto Token::data() const noexcept -> Str {
  return data_;
}

auto Token::loc() const noexcept -> Location {
  return loc_;
}

auto Token::is_none() const noexcept -> bool {
  return tag_ == Tag::None;
}

auto Token::is_eof() const noexcept -> bool {
  return tag_ == Tag::Eof;
}

auto operator<<(std::ostream &os, Token const &tok) -> std::ostream & {
  using Tag = Token::Tag;

  switch (tok.tag()) {
    case Tag::Number: {
      os << "<number: '" << tok.data() << "'>";
    } break;
    case Tag::Ident: {
      os << "<ident: '" << tok.data() << "'>";
    } break;
    case Tag::OpenBlock: {
      os << "<open_block: '{'>";
    } break;
    case Tag::CloseBlock: {
      os << "<close_block: '}'>";
    } break;
    case Tag::String: {
      os << "<string: " << tok.data() << ">";
    } break;

    case Tag::Eof: {
      os << "<Eof>";
    } break;
    case Tag::None: {
      os << "<None>";
    } break;
  }

  return os;
}

Lexer::Lexer(std::istream &is):
  buffer_(is), current_loc_({1, 1}), current_tok_() { }

auto Lexer::peek_char() -> int {
  auto c = buffer_.peek();
  return c == EOF ? -1 : c;
}

auto Lexer::get_char() -> int {
  auto c = buffer_.get();
  if (c == '\n') {
    current_loc_.newline();
  } else {
    current_loc_.nextch();
  }
  return c == EOF ? -1 : c;
}

auto Lexer::is_special_char(uint8_t c) -> bool {
  return c == '{' || c == '}' || c == '`' || c == '#';
}

auto Lexer::peek_tok() -> Token const & {
  using Tag = Token::Tag;
  if (!current_tok_.is_none()) {
    return current_tok_;
  } else {
    auto loc = current_loc_;
    auto c = peek_char();

    auto buff = std::vector<uint8_t>();

    // TODO(ubsan): comments

    if (c == -1) {
      return current_tok_ =
        Token(Tag::Eof, String(buff.data(), buff.size()), current_loc_);
    } else if (std::isspace(c)) {
      get_char();
      while (std::isspace(c = peek_char())) {
        get_char();
      }
      return peek_tok();
    } else if (c == '#') {
      get_char();
      while ((c = peek_char()) != '\n' && c != -1) {
        get_char();
      }
      return peek_tok();
    } else if (std::isdigit(c) || c == '-') {
      buff.push_back(get_char());
      if (c == '-' && std::isspace(peek_char())) {
        return current_tok_ =
          Token(Tag::Ident, String(buff.data(), buff.size()), loc);
      }
      while (!std::isspace(c = peek_char())) {
        if (!std::isdigit(c) && c != '.') {
          std::cerr
            << "invalid character in number"
            << loc << ": `" << char(c) << "` at " << current_loc_ << "\n";
          std::exit(3);
        }
        buff.push_back(get_char());
      }
      return current_tok_ =
        Token(Tag::Number, String(buff.data(), buff.size()), loc);
    } else if (c == '{') {
      buff.push_back(get_char());
      if (!std::isspace(c = peek_char()) && c != -1) {
        std::cerr
          << "after opening a block" << loc << " there must be whitespace\n";
        std::exit(3);
      }
      return current_tok_ =
        Token(Tag::OpenBlock, String(buff.data(), buff.size()), loc);
    } else if (c == '}') {
      buff.push_back(get_char());
      if (!std::isspace(c = peek_char()) && c != -1) {
        std::cerr
          << "after closing a block" << loc << " there must be whitespace\n";
        std::exit(3);
      }
      return current_tok_ =
        Token(Tag::CloseBlock, String(buff.data(), buff.size()), loc);
    } else if (c == '`') {
      get_char();
      while ((c = peek_char()) != -1) {
        if (c == '\\') {
          get_char();
          buff.push_back(get_char());
        } else if (c == '`') {
          get_char();
          break;
        } else {
          buff.push_back(get_char());
        }
      }
      if (!std::isspace(c = peek_char()) && c != -1) {
        std::cerr
          << "after ending a string" << loc
          << " there must be whitespace at " << current_loc_ << "\n";
        std::exit(3);
      }
      return current_tok_ =
        Token(Tag::String, String(buff.data(), buff.size()), loc);
    } else {
      buff.push_back(get_char());
      while (!std::isspace(c = peek_char())) {
        if (is_special_char(c)) {
          std::cerr
            << "special character(" << DbgPrCh{uint8_t(c)}
            << ") in an identifier" << loc << " at " << current_loc_ << "\n";
          std::exit(3);
        }
        buff.push_back(get_char());
      }
      return current_tok_ =
        Token(Tag::Ident, String(buff.data(), buff.size()), loc);
    }
  }
}

auto Lexer::get_tok() -> Token {
  peek_tok();
  if (current_tok_.is_eof()) {
    return current_tok_;
  } else {
    auto ret = std::move(current_tok_);
    current_tok_ = Token();
    return ret;
  }
}

auto Lexer::run() -> void {
  while (!peek_tok().is_eof()) {
    std::cout << get_tok() << "\n";
  }
}
