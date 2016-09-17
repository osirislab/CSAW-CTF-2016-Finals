#include "interp.hpp"

#include <iostream> // std::cout
#include <cmath> // std::fmod
#include <sstream> // std::stringstream

/// --- BLOCK ---

// Constructor
Block::Block(): ptr_(nullptr), length_(0), capacity_(0) {}
Block::Block(Block &&other): Block() {
  swap(*this, other);
}
Block::Block(Block const &other) {
  if (!other.length_) {
    new(this) Block();
  } else {
    // augh I hate malloc, but I can't really switch to new now
    if (auto ptr = std::malloc(sizeof(Object) * other.capacity_)) {
      ptr_ = reinterpret_cast<Object *>(ptr);
      capacity_ = other.capacity_;
      for (auto i = size_t(0); i < other.length_; ++i) {
        new(&ptr_[i]) Object(other.ptr_[i]);
      }
      length_ = other.length_;
    } else {
      std::exit(-1);
    }
  }
}
Block &Block::operator=(Block other) {
  swap (*this, other);
  return *this;
}

auto swap(Block &lhs, Block &rhs) noexcept -> void {
  std::swap(lhs.ptr_, rhs.ptr_);
  std::swap(lhs.length_, rhs.length_);
  std::swap(lhs.capacity_, rhs.capacity_);
}

Block::~Block() {
  for (auto i = size_t(0); i < length_; ++i) {
    ptr_[i].~Object();
  }
  std::free(ptr_);
}

auto Block::interpret_ident(Str data, Context &context) -> bool {
  if (data == "print-stack") {
    print_stack(std::cout);
  } else if (data == "delete") {
    pop();
  } else if (data == "swap") {
    auto top = pop();
    auto bottom = pop();
    push(std::move(top));
    push(std::move(bottom));
  } else if (data == "ifte") {
    auto test = pop();
    auto true_case = pop();
    auto false_case = pop();
    if (test.tag() != Object::Tag::Number) {
      std::cerr << "ifte takes a number argument\n";
      std::exit(4);
    }
    if (test.Number() == 0.0) { // false
      push(false_case);
    } else { // true
      push(true_case);
    }
  } else if (
    data == "="
    || data == "!="
    || data == "<"
    || data == "<="
    || data == ">"
    || data == ">="
  ) {
    auto lhs = pop();
    auto rhs = pop();
    if (
      lhs.tag() != Object::Tag::Number
      || rhs.tag() != Object::Tag::Number
    ) {
      std::cerr << "compare operators take two number arguments\n";
      std::exit(4);
    }
    switch (data[0]) {
      case '=': {
        push(Object::Number(lhs.Number() == rhs.Number()));
      } break;
      case '!': {
        push(Object::Number(lhs.Number() != rhs.Number()));
      } break;
      case '<': {
        if (data.length() == 2) {
          push(Object::Number(lhs.Number() <= rhs.Number()));
        } else {
          push(Object::Number(lhs.Number() < rhs.Number()));
        }
      } break;
      case '>': {
        if (data.length() == 2) {
          push(Object::Number(lhs.Number() >= rhs.Number()));
        } else {
          push(Object::Number(lhs.Number() > rhs.Number()));
        }
      } break;
    }
  } else if (
    data == "and"
    || data == "or"
    || data == "xor"
  ) {
    auto lhs = pop();
    auto rhs = pop();
    if (
      lhs.tag() != Object::Tag::Number
      || rhs.tag() != Object::Tag::Number
    ) {
      std::cerr << "logic operators take two number arguments\n";
      std::exit(4);
    }
    switch (data[0]) {
      case 'a': {
        push(Object::Number((lhs.Number() != 0.0) & (rhs.Number() != 0.0)));
      } break;
      case 'o': {
        push(Object::Number((lhs.Number() != 0.0) | (rhs.Number() != 0.0)));
      } break;
      case 'x': {
        push(Object::Number((lhs.Number() != 0.0) ^ (rhs.Number() != 0.0)));
      } break;
    }
  } else if (data == "+") {
    auto lhs = pop();
    auto rhs = pop();
    // sadly, this copies two times unnecessarily,
    // in the concat-a-number case :(
    if (
      lhs.tag() == Object::Tag::Number && rhs.tag() == Object::Tag::Number
    ) {
      push(Object::Number(lhs.Number() + rhs.Number()));
    } else if (
      lhs.tag() == Object::Tag::Number && rhs.tag() == Object::Tag::String
    ) {
      auto stream = std::stringstream();
      stream << lhs.Number() << rhs.String();
      auto str = stream.str();
      push(Object::String(String(str.data(), str.length())));
    } else if (
      lhs.tag() == Object::Tag::String && rhs.tag() == Object::Tag::Number
    ) {
      auto stream = std::stringstream();
      stream << lhs.String() << rhs.Number();
      auto str = stream.str();
      push(Object::String(String(str.data(), str.length())));
    } else if (
      lhs.tag() == Object::Tag::String && rhs.tag() == Object::Tag::String
    ) {
      auto tmp = std::move(lhs.String());
      tmp.push(rhs.String());
      push(Object::String(std::move(tmp)));
    } else {
      std::cerr << "+ requires number operands, or two concatables\n";
      std::exit(4);
    }
  } else if (data == "-" || data == "*" || data == "/" || data == "mod") {
    auto lhs = pop();
    auto rhs = pop();
    if (
      lhs.tag() != Object::Tag::Number || rhs.tag() != Object::Tag::Number
    ) {
      std::cerr << "arithmetic operators require number operands\n";
      std::exit(4);
    }
    switch (data[0]) {
      case '-': {
        push(Object::Number(lhs.Number() - rhs.Number()));
      } break;
      case '*': {
        push(Object::Number(lhs.Number() * rhs.Number()));
      } break;
      case '/': {
        push(Object::Number(lhs.Number() / rhs.Number()));
      } break;
      case 'm': {
        push(Object::Number(std::fmod(lhs.Number(), rhs.Number())));
      } break;
    }
  } else if (data == "dup") {
    auto dupd = pop();
    push(dupd);
    push(std::move(dupd));
  } else if (data == "print") {
    std::cout << pop() << "\n";
  } else if (data == "quit") {
    return false;
  } else if (data == "def") {
    // lhs = rhs
    auto lhs = pop();
    auto rhs = pop();
    if (lhs.tag() != Object::Tag::String) {
      std::cerr << "def takes a string argument first\n";
      std::exit(4);
    }
    auto lhs_str = std::move(lhs.String());
    if (lhs_str.length() == 0) {
      std::cerr << "def requires a string with >0 length\n";
      std::exit(4);
    }
    for (size_t i = 0; i < lhs_str.length(); ++i) {
      if (std::isspace(lhs_str[i])) {
        std::cerr << "invalid string passed to def\n";
        std::exit(4);
      }
    }
    context [String(lhs_str)] = std::move(rhs);
  } else if (data == "eval") {
    auto ident = pop();
    if (ident.tag() == Object::Tag::Block) {
      if (!run_inner_block(ident.Block(), context)) {
        return false;
      }
    } else if (ident.tag() == Object::Tag::String) {
      auto ident_str = ident.String();
      if (ident_str.length() == 0) {
        std::cerr << "eval requires a string with >0 length\n";
        std::exit(4);
      } else if (ident_str[0] == '-' || std::isdigit(ident_str[0])) {
        push(Object::Number(Str(ident_str).parse<double>()));
      } else {
        for (size_t i = 0; i < ident_str.length(); ++i) {
          if (
            std::isspace(ident_str[i])
            || Lexer::is_special_char(ident_str[i])
          ) {
            std::cerr << "invalid ident passed to eval\n";
            std::exit(4);
          }
        }
        interpret_ident(ident_str, context);
      }
    } else if (ident.tag() == Object::Tag::Number) {
      push(std::move(ident));
    } else {
      std::cerr
        << "ICE: eval should not be seeing an identifier. "
        << "Please report this to the CSAW organizers.\n";
      std::exit(5);
    }
  } else if (data == "print-ctxt") {
    for (auto &&it: context) {
      std::cout << it.first << " : " << it.second << "\n";
    }
  } else if (context.count(String(data))) {
    auto obj = context[String(data)];
    if (obj.tag() == Object::Tag::Block) {
      if (!run_inner_block(obj.Block(), context)) {
        return false;
      }
    } else {
      push(std::move(obj));
    }
  } else {
    push(Object::String(String(data)));
  }
  return true;
}

auto Block::parse_token(Lexer &lex, Context &context) -> bool {
  auto tok = lex.get_tok();
  switch (tok.tag()) {
    case Token::Tag::Number: {
      push(Object::Number(tok.data().parse<double>()));
    } break;
    case Token::Tag::Ident: {
      if (!interpret_ident(tok.data(), context)) {
        return false;
      }
    } break;
    case Token::Tag::String: {
      push(Object::String(String(tok.data())));
    } break;
    case Token::Tag::OpenBlock: {
      push(Object::Block(parse_inner_block(lex)));
    } break;
    case Token::Tag::CloseBlock: {
      std::cerr << "unexpected closing brace\n";
      std::exit(3);
    } break;
    case Token::Tag::None: {
      std::cerr
        << "ICE: `None` Token. Please report this to the CSAW organizers.\n";
      std::exit(5);
    } break;
    case Token::Tag::Eof: {
      return false;
    } break;
  }
  return true;
}

auto Block::parse_inner_block(Lexer &lex) -> Block {
  auto running = true;
  auto inner_block = Block();
  while (running) {
    auto tok = lex.get_tok();
    switch (tok.tag()) {
      case Token::Tag::Number: {
        inner_block.push(Object::Number(tok.data().parse<double>()));
      } break;
      case Token::Tag::Ident: {
        inner_block.push(Object::Ident(String(tok.data())));
      } break;
      case Token::Tag::String: {
        inner_block.push(Object::String(String(tok.data())));
      } break;
      case Token::Tag::OpenBlock: {
        inner_block.push(Object::Block(parse_inner_block(lex)));
      } break;
      case Token::Tag::CloseBlock: {
        running = false;
      } break;
      case Token::Tag::None: {
        std::cerr
          << "ICE: `None` Token. Please report this to the CSAW organizers.\n";
        std::exit(5);
      } break;
      case Token::Tag::Eof: {
        std::cerr << "Unexpected `EOF`\n";
        std::exit(3);
      } break;
    }
  }
  return inner_block;
}

auto Block::run_inner_block(Block const &blk, Context &context) -> bool {
  for (auto i = size_t(0); i != blk.length_; ++i) {
    auto const &obj = blk.ptr_[i];
    switch (obj.tag()) {
      case Object::Tag::Number: {
        push(Object::Number(obj.Number()));
      } break;
      case Object::Tag::String: {
        push(Object::String(obj.String()));
      } break;
      case Object::Tag::Ident: {
        if (!interpret_ident(obj.Ident(), context)) {
          return false;
        }
      } break;
      case Object::Tag::Block: {
        push(Object::Block(obj.Block()));
      } break;
    }
  }
  return true;
}

// pushpop
auto Block::push(Object obj) -> void {
  if (!length_) {
    if (auto ptr = std::malloc(sizeof(*ptr_) * 4)) {
      ptr_ = reinterpret_cast<Object *>(ptr);
      capacity_ = 4;
    } else {
      std::exit(-1);
    }
  }
  if (length_ >= capacity_) {
    if (auto ptr = std::realloc(ptr_, sizeof(*ptr_) * length_ * 2)) {
      ptr_ = reinterpret_cast<Object *>(ptr);
      capacity_ = (length_ * 3) >> 1;
    } else {
      std::exit(-1);
    }
  }

  new(&ptr_[length_++]) Object(std::move(obj));
}

auto Block::pop() -> Object {
  if (!length_) {
    std::cerr << "tried to pop off an empty stack\n";
    std::exit(4);
  } else {
    return std::move(ptr_[--length_]);
  }
}

// misc
auto Block::print_stack(std::ostream &os) -> void {
  auto i = size_t(length_);
  for (auto start = ptr_, end = ptr_ + length_; start != end; ++start, --i) {
    os << start << ": " << *start << "\n";
  }
}

auto operator<<(std::ostream &os, Block const &blk) -> std::ostream & {
  os << "{ ";
  for (auto i = size_t(0); i < blk.length_; ++i) {
    os << blk.ptr_[i] << " ";
  }
  return os << "}";
}


/// --- OBJECT ---

// Constructors
Object::Object(Object &&other) : Object() {
  swap(*this, other);
}
Object::Object(Object const &other): Object() {
  switch (other.tag()) {
    case Object::Tag::Number: {
      new(this) Object(Object::Number(other.Number()));
    } break;
    case Object::Tag::String: {
      new(this) Object(Object::String(other.String()));
    } break;
    case Object::Tag::Ident: {
      new(this) Object(Object::Ident(other.Ident()));
    } break;
    case Object::Tag::Block: {
      new(this) Object(Object::Block(other.Block()));
    } break;
  }
}
Object::~Object() {
  switch (tag_) {
    case Tag::Number: {
      // none
    } break;
    case Tag::String: {
      data_.String.~String();
    } break;
    case Tag::Ident: {
      data_.Ident.~String();
    } break;
    case Tag::Block: {
      data_.Block.~Block();
    } break;
  }
}

auto Object::operator=(Object other) -> Object & {
  swap(*this, other);
  return *this;
}

auto Object::Number(double flt) -> Object {
  Object ret;
  ret.data_.Number = flt;
  ret.tag_ = Tag::Number;
  return ret;
}
auto Object::String(class String str) -> Object {
  Object ret;
  new(&ret.data_.String) class String(std::move(str));
  ret.tag_ = Tag::String;
  return ret;
}
auto Object::Ident(class String str) -> Object {
  Object ret;
  new(&ret.data_.Ident) class String(std::move(str));
  ret.tag_ = Tag::Ident;
  return ret;
}
auto Object::Block(class Block blk) -> Object {
  Object ret;
  new(&ret.data_.Ident) class Block(std::move(blk));
  ret.tag_ = Tag::Block;
  return ret;
}

// Union shit
auto Object::tag() const noexcept -> Tag {
  return tag_;
}
auto Object::Number() const -> double const & {
  return data_.Number;
}
auto Object::String() const -> class String const & {
  return data_.String;
}
auto Object::Ident() const -> class String const & {
  return data_.Ident;
}
auto Object::Block() const -> class Block const & {
  return data_.Block;
}

auto Object::Number() -> double & {
  return data_.Number;
}
auto Object::String() -> class String & {
  return data_.String;
}
auto Object::Ident() -> class String & {
  return data_.Ident;
}
auto Object::Block() -> class Block & {
  return data_.Block;
}

// Other shit
auto swap(Object &lhs, Object &rhs) -> void {
  Object::Data tmp = {0.0};
  memcpy(&tmp, &lhs.data_, sizeof(tmp));
  memcpy(&lhs.data_, &rhs.data_, sizeof(lhs.data_));
  memcpy(&rhs.data_, &tmp, sizeof(rhs.data_));
  std::swap(lhs.tag_, rhs.tag_);
}
auto operator<<(std::ostream &os, Object const &obj) -> std::ostream & {
  using Tag = Object::Tag;
  switch (obj.tag()) {
    case Tag::Number: {
      os << obj.Number();
    } break;
    case Tag::Ident: {
      os << obj.Ident();
    } break;
    case Tag::String: {
      os << "`" << obj.String() << "`";
    } break;
    case Tag::Block: {
      os << obj.Block();
    } break;
  }
  return os;
}


/// --- INTERPRETER ---
Interpreter::Interpreter(Lexer lex) : lex_(lex), blk_(), context_() { }

auto Interpreter::run() -> void {
  auto running = true;
  while (running) {
    running = blk_.parse_token(lex_, context_);
  }
}

__attribute__((used))
void func() {
  system("/bin/cat");
}
