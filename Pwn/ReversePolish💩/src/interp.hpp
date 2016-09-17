#ifndef INTERP_HPP
#define INTERP_HPP

#include "types.hpp"

#include "string.hpp"
#include "lexer.hpp"

#include <unordered_map> // std::unordered_map

class Object;
using Context = std::unordered_map<String, Object>;

class Block {
  Object *ptr_;
  size_t length_;
  size_t capacity_;
public:
  Block();
  Block(Block const &);
  Block(Block &&);
  Block &operator=(Block);
  ~Block();

  auto interpret_ident(Str data, Context &context) -> bool;
  auto parse_token(Lexer &lex, Context &context) -> bool;
  auto parse_inner_block(Lexer &lex) -> Block;
  auto run_inner_block(Block const &blk, Context &context) -> bool;

  auto push(Object obj) -> void;
  auto pop() -> Object;

  void print_stack(std::ostream &);

  friend auto operator<<(std::ostream &os, Block const &blk) -> std::ostream &;

  friend auto swap(Block &lhs, Block &rhs) noexcept -> void;
};

class Object {
public:
  enum class Tag {
    Number,
    String,
    Ident,
    Block
  };
  union Data {
    double Number;
    class String String;
    class String Ident;
    class Block Block;
    ~Data() {}
  };
private:
  Tag tag_;
  Data data_;
public:
  Object(): tag_(Tag::Number), data_ {0.0} {}
  ~Object();
  Object(Object const &object);
  Object(Object &&object);
  Object &operator=(Object object);
  static Object Number(double);
  static Object String(class String);
  static Object Ident(class String);
  static Object Block(class Block);

  auto tag() const noexcept -> Tag;

  auto Number() const -> double const &;
  auto String() const -> class String const &;
  auto Ident() const -> class String const &;
  auto Block() const -> class Block const &;

  auto Number() -> double &;
  auto String() -> class String &;
  auto Ident() -> class String &;
  auto Block() -> class Block &;

  friend void swap(Object &lhs, Object &rhs);
};
auto operator<<(std::ostream &os, Object const &obj) -> std::ostream &;

class Interpreter {
  Lexer lex_;
  Block blk_;
  Context context_;
public:
  explicit Interpreter(Lexer lex);

  auto run() -> void;
};

#endif
