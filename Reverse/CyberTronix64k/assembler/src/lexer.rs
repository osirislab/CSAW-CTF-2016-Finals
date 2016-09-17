use std::collections::HashMap;
use std::cell::RefCell;
use std::rc::Rc;
use std::fmt::{self, Display};
use std::path::Path;

// index into the file vector
#[derive(Copy, Clone, PartialEq)]
pub struct File(u32);
#[derive(Clone)]
pub struct __Files {
  vec: Rc<RefCell<Vec<String>>>,
  hm: Rc<RefCell<HashMap<String, u32>>>,
}

impl PartialEq for __Files {
  fn eq(&self, other: &Self) -> bool {
    &*self.hm as *const RefCell<_> == &*other.hm as *const RefCell<_>
  }
}
impl Eq for __Files {}
#[derive(Clone, PartialEq, Eq)]
pub struct Files(Option<__Files>);

impl Files {
  pub fn new() -> Self {
    let ret = Files(Some(__Files {
      vec: Rc::new(RefCell::new(Vec::new())),
      hm: Rc::new(RefCell::new(HashMap::new())),
    }));
    assert!(ret.push("<compiler-defined>") == File(0));
    ret
  }
  pub fn empty() -> Self {
    Files(None)
  }
  pub fn get(&self, file: File) -> Option<&str> {
    // the reason this is safe is because the Strings in the vector
    // aren't deallocated until all Files objects are deallocated
    let this = self.0
      .as_ref()
      .expect("ICE: Attempted to get a file from an empty Files");
    unsafe {
      this.vec
        .borrow()
        .get(file.0 as usize)
        .map(|x| &*(&**x as *const str))
    }
  }

  pub fn push(&self, s: &str) -> File {
    let this =
      self.0.as_ref().expect("ICE: Attempted to push a file to an empty Files");
    let mut hm = this.hm.borrow_mut();
    let mut vec = this.vec.borrow_mut();
    match hm.get(s) {
      Some(&f) => File(f),
      None => {
        let f = vec.len() as u32;
        hm.insert(s.to_owned(), f);
        vec.push(s.to_owned());
        File(f)
      },
    }
  }
}

#[derive(Clone)]
pub struct Position {
  pub line: usize,
  pub offset: usize,
  file: File,
  files: Files,
}

impl Position {
  // if you don't want any data
  pub fn empty() -> Self {
    Position {
      line: 0,
      offset: 0,
      file: File(0),
      files: Files::empty(),
    }
  }

  #[allow(unused)]
  pub fn file(&self) -> &str {
    self.files.get(self.file).unwrap()
  }

  fn new(filename: &str, files: Files) -> Self {
    let file = files.push(filename);
    Position {
      line: 1,
      offset: 1,
      file: file,
      files: files,
    }
  }

  fn newline(&mut self) {
    self.line += 1;
    self.offset = 0;
  }
  fn next(&mut self) {
    self.offset += 1;
  }
}

impl Display for Position {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
    write!(
      f,
      "{}:{}:{}",
      self.files.get(self.file).unwrap(),
      self.line,
      self.offset,
    )
  }
}

#[derive(Clone)]
pub struct OpArg {
  pub var: OpArgVar,
  pub pos: Position,
}

impl OpArg {
  pub fn evaluate(
    &self, labels: &HashMap<String, u16>, mac_args: &[OpArg], inst_offset: u16,
  ) -> u16 {
    match self.var {
      OpArgVar::Number(n) => n,
      OpArgVar::Label(ref label) => match labels.get(label) {
        Some(&n) => n,
        None => error!(self.pos, "Undefined label: {}", label),
      },
      OpArgVar::MacroArg(n) =>
        mac_args[n as usize].evaluate(labels, &[], inst_offset),
      OpArgVar::ArithOp(ref op, ref lhs, ref rhs) => op.op(
        lhs.evaluate(labels, mac_args, inst_offset),
        rhs.evaluate(labels, mac_args, inst_offset),
      ),
      OpArgVar::Here => inst_offset,
    }
  }
}

#[derive(Copy, Clone)]
pub enum ArithOp {
  Add,
  #[allow(dead_code)]
  Sub,
  #[allow(dead_code)]
  Mul,
  #[allow(dead_code)]
  Div,
}

impl ArithOp {
  pub fn op(self, lhs: u16, rhs: u16) -> u16 {
    match self {
      ArithOp::Add => lhs + rhs,
      ArithOp::Sub => lhs - rhs,
      ArithOp::Mul => lhs * rhs,
      ArithOp::Div => lhs / rhs,
    }
  }
}

#[derive(Clone)]
pub enum OpArgVar {
  Number(u16),
  Label(String),
  MacroArg(u16),
  ArithOp(ArithOp, Box<OpArg>, Box<OpArg>),
  Here, // $
}

#[derive(Copy, Clone)]
pub enum Public {
  Public,
  Private,
}

#[derive(Clone)]
pub enum DirectiveVar {
  Public(String), // TODO(ubsan): useless for now
  Label(String, Public),
  Import(Vec<String>, Public),
  Const(String, OpArg, Public),
  Op(String, Vec<OpArg>),
  // TODO(ubsan): allow non-constant reps?
  Data(Vec<OpArg>),
  #[allow(dead_code)]
  Macro {
    name: String,
    args: u16,
    expansions: Vec<(String, Vec<OpArg>)>,
  },
}

#[derive(Clone)]
pub struct Directive {
  pub var: DirectiveVar,
  pub pos: Position,
}

enum TokenVar {
  Ident(Vec<u8>),
  Label(Vec<u8>),
  StrLit(Vec<u8>),
  NumLit(u16),
  MacroArg(u16),
  MacroLabel(Vec<u8>),
  Data,
  Equ,
  Rep,
  Macro,
  EndMacro,
  Import,
  Public,
  Here, // $
  Dot,
  Comma,
  Newline,
}

impl Display for TokenVar {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
    match *self {
      TokenVar::Ident(_) => write!(f, "identifier"),
      TokenVar::Label(_) => write!(f, "label"),
      TokenVar::Newline => write!(f, "newline"),
      TokenVar::Comma => write!(f, "comma"),
      TokenVar::Dot => write!(f, "period"),
      TokenVar::Here => write!(f, "$"),
      TokenVar::Equ => write!(f, "equ directive"),
      TokenVar::Import => write!(f, "import directive"),
      TokenVar::Public => write!(f, "public directive"),
      TokenVar::Data => write!(f, "data directive"),
      TokenVar::Rep => write!(f, "rep directive"),
      TokenVar::Macro => write!(f, "macro directive"),
      TokenVar::EndMacro => write!(f, "endmacro directive"),
      TokenVar::StrLit(_) => write!(f, "string literal"),
      TokenVar::NumLit(_) => write!(f, "number literal"),
      TokenVar::MacroLabel(_) => write!(f, "macro label"),
      TokenVar::MacroArg(_) => write!(f, "macro argument"),
    }
  }
}

struct Token {
  var: TokenVar,
  pos: Position,
}

pub struct Lexer {
  input: Vec<u8>,
  idx: usize,

  files: Files,
  pos: Position,
}
// UTILITY
impl Lexer {
  fn get_bytes(filename: &Path) -> Vec<u8> {
    use std::io::Read;
    let mut file = match ::std::fs::File::open(filename) {
      Ok(file) => file,
      Err(e) => error_np!(
        "Failed to open input file: `{}'\nError: {}",
        filename.to_str().unwrap(),
        e,
      ),
    };

    let mut bytes = Vec::new();
    match file.read_to_end(&mut bytes) {
      Ok(_) => {},
      Err(e) => error_np!(
        "Failed to read file: `{}'\nError: {}", filename.to_str().unwrap(), e,
      ),
    };
    bytes
  }
  pub fn new(filename: &Path) -> Self {
    let bytes = Self::get_bytes(filename);
    let files = Files::new();
    let pos = Position::new(filename.to_str().unwrap(), files.clone());
    Lexer {
      input: bytes,
      idx: 0,
      files: files,
      pos: pos,
    }
  }

  pub fn new_file_lexer(&self, filename: &Path) -> Self {
    let bytes = Self::get_bytes(filename);
    let files = self.files.clone();
    let pos = Position::new(filename.to_str().unwrap(), files.clone());
    Lexer {
      input: bytes,
      idx: 0,
      files: files,
      pos: pos,
    }
  }

  pub fn compiler_defined_pos(&self) -> Position {
    Position {
      line: 0,
      offset: 0,
      file: File(0),
      files: self.files.clone()
    }
  }

  fn pos(&self) -> Position {
    self.pos.clone()
  }
}

// TOKENIZATION
impl Lexer {
  fn peek_char(&self) -> Option<u8> {
    self.input.get(self.idx).cloned()
  }

  fn get_char(&mut self) -> Option<(u8, Position)> {
    match self.peek_char() {
      Some(c) => {
        self.idx += 1;
        let pos = self.pos.clone();
        if c == b'\n' {
          self.pos.newline();
        } else {
          self.pos.next();
        }
        Some((c, pos))
      }
      None => None,
    }
  }

  fn block_comment(&mut self) {
    while let Some((c, _)) = self.get_char() {
      if c == b'-' {
        if let Some(b'#') = self.peek_char() {
          self.get_char();
          return;
        }
      } else if c == b'#' {
        if let Some(b'-') = self.peek_char() {
          self.get_char();
          self.block_comment();
        }
      }
    }
    error!(self.pos, "Unexpected EOF");
  }

  fn next_token(&mut self) -> Option<Token> {
    fn is_space(c: u8) -> bool {
      c == b' ' || c == b'\t' || c == 0x0b || c == 0x0c  || c == b'\r'
    }
    fn is_alpha(c: u8) -> bool {
      (c >= b'a' && c <= b'z') || (c >= b'A' && c <= b'Z')
    }
    fn is_ident_start(c: u8) -> bool {
      is_alpha(c) || c == b'_'
    }
    fn is_num(c: u8) -> bool {
      c >= b'0' && c <= b'9'
    }
    fn is_ident(c: u8) -> bool {
      is_ident_start(c) || is_num(c)
    }
    fn is_allowed(c: u8, base: u16) -> bool {
      match base {
        2 => c >= b'0' && c < b'2',
        8 => c >= b'0' && c < b'8',
        10 => is_num(c),
        16 => is_num(c) || (c >= b'A' && c <= b'F') || (c >= b'a' && c <= b'f'),
        _ => unreachable!(),
      }
    }

    if let Some((ch, pos)) = self.get_char() {
      match ch {
        b'\\' => {
          let ch = self.get_char();
          if let Some((b'\n', _)) = ch {
            self.next_token()
          } else if let Some((ch, pos)) = ch {
            error!(pos, "Unexpected `{}' ({})", ch as char, ch)
          } else {
            error!(self.pos, "Unexpected EOF")
          }
        },
        ch if ch == b'#' || ch == b';' => {
          if let Some(c)  = self.peek_char() {
            if ch == b'#' && c == b'-' {
              self.get_char();
              self.block_comment();
              return self.next_token();
            }
          }
          while let Some(c) = self.peek_char() {
            if c != b'\n' { self.get_char(); }
            else { break; }
          }
          self.next_token()
        },
        ch if is_space(ch) => {
          while let Some(c) = self.peek_char() {
            if is_space(c) { self.get_char(); }
            else { break; }
          }
          self.next_token()
        },
        b'\n' => Some(Token {
          var: TokenVar::Newline,
          pos: pos,
        }),
        b',' => Some(Token {
          var: TokenVar::Comma,
          pos: pos,
        }),
        b'$' => Some(Token {
          var: TokenVar::Here,
          pos: pos,
        }),
        quote if quote == b'\'' || quote == b'"' => {
          let mut buff = Vec::new();
          while let Some(ch) = self.get_char() {
            if ch.0 == b'\\' {
              match self.get_char() {
                Some((b'\'', _)) => buff.push(b'\''),
                Some((b'"', _)) => buff.push(b'"'),
                Some((b'\n', _)) => {
                  loop {
                    if let Some(c) = self.peek_char() {
                      if is_space(c) {
                        self.get_char();
                      } else {
                        break;
                      }
                    } else {
                      error!(self.pos, "Unexpected EOF")
                    }
                  }
                },
                Some((b'n', _)) => buff.push(b'\n'),
                Some((ch, pos)) => error!(
                  pos, "Unrecogized escape sequence: \\{}", ch,
                ),
                None => error!(self.pos, "Unexpected EOF"),
              }
            } else if ch.0 == quote {
              break;
            } else {
              buff.push(ch.0);
            }
          }
          Some(Token {
            var: TokenVar::StrLit(buff),
            pos: pos,
          })
        },
        ch if is_ident_start(ch) => {
          let mut ret = Vec::new();
          ret.push(ch);
          while let Some(c) = self.peek_char() {
            if is_ident(c) {
              self.get_char();
              ret.push(c);
            } else if c == b':' {
              self.get_char();
              return Some(Token {
                var: TokenVar::Label(ret),
                pos: pos,
              });
            } else {
              break;
            }
          }
          Some(Token {
            var: {
              if ret == b"data" {
                TokenVar::Data
              } else if ret == b"equ" {
                TokenVar::Equ
              } else if ret == b"rep" {
                TokenVar::Rep
              } else if ret == b"macro" {
                TokenVar::Macro
              } else if ret == b"endmacro" {
                TokenVar::EndMacro
              } else if ret == b"import" {
                TokenVar::Import
              } else if ret == b"public" {
                TokenVar::Public
              } else {
                TokenVar::Ident(ret)
              }
            },
            pos: pos,
          })
        }
        ch if is_num(ch) => {
          let mut base = 10;
          let mut ret = Vec::new();
          if ch == b'0' {
            match self.peek_char() {
              Some(b'b') => base = 2,
              Some(b'o') => base = 8,
              Some(b'd') => base = 10,
              Some(b'x') => base = 16,
              Some(ch) if is_num(ch) => ret.push(ch),
              Some(ch) if is_alpha(ch) =>
                error!(self.pos, "Unknown base specifier: {}", ch as char),
              Some(_) | None => return Some(Token {
                var: TokenVar::NumLit(0),
                pos: pos,
              })
            }
            self.get_char();
          } else {
            ret.push(ch);
          }

          while let Some(ch) = self.peek_char() {
            if is_allowed(ch, base) {
              self.get_char();
              ret.push(ch);
            } else if is_alpha(ch) {
              error!(
                self.pos,
                "Unsupported character in a base-{} literal: {}",
                base,
                ch as char,
              );
            } else {
              break;
            }
          }
          let ret = ret.iter().fold(0, |acc: u16, &el: &u8| {
            let add = if is_alpha(el) { el - b'A' + 10 } else { el - b'0' };
            match acc.checked_mul(base).and_then(|a| a.checked_add(add as u16)) {
              Some(a) => a,
              None => error!(
                pos,
                "Attempted to write an overflowing number literal: {}",
                ::std::str::from_utf8(&ret).unwrap(),
              ),
            }
          });
          Some(Token {
            var: TokenVar::NumLit(ret),
            pos: pos,
          })
        },
        b'%' => {
          if let Some(next_tok) = self.next_token() {
            if let TokenVar::NumLit(n) = next_tok.var {
              Some(Token {
                var: TokenVar::MacroArg(n),
                pos: self.pos(),
              })
            } else if let TokenVar::Label(label) = next_tok.var {
              Some(Token {
                var: TokenVar::MacroLabel(label),
                pos: pos,
              })
            } else if let TokenVar::Ident(_) = next_tok.var {
              error!(self.pos, "Expected a colon");
            } else {
              error!(next_tok.pos, "Expected a label or number");
            }
          } else {
            error!(self.pos, "Unexpected EOF");
          }
        }
        b'.' => Some(Token { var: TokenVar::Dot, pos: pos }),
        ch => error!(
          pos, "Unsupported character: `{}' (0x{:X})", ch as char, ch,
        ),
      }
    } else {
      None
    }
  }
}

// LEXING
impl Lexer {
  fn to_string(pos: &Position, v: Vec<u8>) -> String {
    match String::from_utf8(v) {
      Ok(s) => s,
      Err(_) => error!(pos, "Invalid utf8"),
    }
  }

  // None means EOL
  fn get_op_arg(tok: Token) -> Option<OpArg> {
    match tok.var {
      TokenVar::Newline => None,
      TokenVar::Here => Some(OpArg {
        var: OpArgVar::Here,
        pos: tok.pos,
      }),
      TokenVar::Ident(id) => Some(OpArg {
        var: OpArgVar::Label(Self::to_string(&tok.pos, id)),
        pos: tok.pos,
      }),
      TokenVar::NumLit(n) => Some(OpArg {
        var: OpArgVar::Number(n),
        pos: tok.pos,
      }),
      TokenVar::StrLit(s) => {
        if s.len() == 1 {
          Some(OpArg {
            var: OpArgVar::Number(s[0] as u16),
            pos: tok.pos
          })
        } else if s.is_empty() {
          error!(
            tok.pos, "Unexpected empty string literal",
          );
        } else {
          error!(
            tok.pos, "Unexpected multi-char string literal",
          );
        }
      }
      tv => error!(tok.pos, "Unexpected {}", tv),
    }
  }

  fn dir_label(&mut self, pos: Position, label: Vec<u8>) -> Directive {
    Directive {
      var: DirectiveVar::Label(
        Self::to_string(&pos, label), Public::Private,
      ),
      pos: pos,
    }
  }

  fn dir_ident(&mut self, pos: Position, op: Vec<u8>) -> Directive {
    let op = Self::to_string(&pos, op);
    let mut args = Vec::new();
    while let Some(tok) = self.next_token() {
      let pos = tok.pos.clone();
      if let TokenVar::Newline = tok.var {
        break;
      } else if let Some(arg) = Self::get_op_arg(tok) {
        args.push(arg);
        if let Some(tok) = self.next_token() {
          if let TokenVar::Newline = tok.var {
            break;
          } else if let TokenVar::Comma = tok.var {
          } else {
            error!(tok.pos, "Expected a comma or a newline");
          }
        } else {
          break;
        }
      } else {
        error!(pos, "Expected an argument or a newline");
      }
    }
    Directive {
      var: DirectiveVar::Op(op, args),
      pos: pos,
    }
  }

  fn dir_data(&mut self, pos: Position) -> Directive {
    let mut data = Vec::new();
    let mut must_get = true;
    while let Some(tok) = self.next_token() {
      match tok.var {
        TokenVar::Rep => {
          let repetitions = match self.next_token() {
            Some(tok) => match tok.var {
              TokenVar::NumLit(n) => n,
              _ => error!(
                tok.pos, "Expected literal number of repetitions",
              ),
            },
            None => error!(tok.pos, "Unexpected EOF"),
          };
          match self.next_token() {
            Some(tok) => match Self::get_op_arg(tok) {
              Some(op) => for _ in 0..repetitions {
                data.push(op.clone());
              },
              None => error!(self.pos, "Unexpected newline"),
            },
            None => error!(self.pos, "Unexpected EOF"),
          }
          must_get = false;
        }
        TokenVar::Ident(id) => {
          data.push(OpArg {
            var: OpArgVar::Label(Self::to_string(&tok.pos, id)),
            pos: tok.pos,
          });
          must_get = false;
        },
        TokenVar::StrLit(s) => {
          let pos = tok.pos;
          data.extend(s.into_iter().map(|c| OpArg {
            var: OpArgVar::Number(c as u16),
            pos: pos.clone(),
          }));
          must_get = false;
        },
        TokenVar::NumLit(n) => {
          data.push(OpArg {
            var: OpArgVar::Number(n),
            pos: tok.pos,
          });
          must_get = false;
        },
        TokenVar::Here => {
          data.push(OpArg {
            var: OpArgVar::Here,
            pos: tok.pos,
          });
          must_get = false;
        },
        TokenVar::Comma => {
          if must_get {
            error!(tok.pos, "Unexpected comma");
          }
          must_get = true;
        },
        TokenVar::Newline => {
          if must_get {
            error!(tok.pos, "Unexpected newline");
          }
          break
        }
        tv => error!(tok.pos, "Unexpected {}", tv),
      }
    }
    Directive {
      var: DirectiveVar::Data(data),
      pos: pos,
    }
  }

  fn dir_public(&mut self, pos: Position) -> Directive {
    if let Some(tok) = self.next_token() {
      match tok.var {
        TokenVar::Label(label) => {
          let mut ret = self.dir_label(tok.pos, label);
          if let DirectiveVar::Label(_, ref mut public) = ret.var {
            *public = Public::Public;
          } else { panic!("ICE: dir_label didn't return a label"); }
          ret
        },
        TokenVar::Ident(op) => Directive {
          var: DirectiveVar::Public(Self::to_string(&pos, op)),
          pos: pos,
        },
        TokenVar::Import => {
          let mut ret = self.dir_import(tok.pos);
          if let DirectiveVar::Import(_, ref mut public) = ret.var {
            *public = Public::Public;
          } else { panic!("ICE: dir_import didn't return an import"); }
          ret
        },
        TokenVar::Equ => {
          let mut ret = self.dir_equ(tok.pos);
          if let DirectiveVar::Const(_, _, ref mut public) = ret.var {
            *public = Public::Public;
          } else { panic!("ICE: dir_equ didn't return an equ"); }
          ret
        },
        tv => error!(tok.pos, "Unexpected {}", tv),
      }
    } else {
      error!(self.pos, "Unexpected EOF");
    }
  }

  fn dir_import(&mut self, pos: Position) -> Directive {
    if let Some(mut tok) = self.next_token() {
      let mut parts = Vec::new();
      loop {
        match tok.var {
          TokenVar::Ident(s) => parts.push(Self::to_string(&tok.pos, s)),
          tv => error!(tok.pos, "Unexpected {}", tv),
        }
        if let Some(tok) = self.next_token() {
          match tok.var {
            TokenVar::Dot => {},
            TokenVar::Newline => return Directive {
              var: DirectiveVar::Import(parts, Public::Private),
              pos: pos,
            },
            tv => error!(tok.pos, "Unexpected {}", tv),
          }
        } else {
          return Directive {
            var: DirectiveVar::Import(parts, Public::Private),
            pos: pos,
          };
        }
        if let Some(new_tok) = self.next_token() {
          tok = new_tok;
        } else {
          error!(self.pos, "Unexpected EOF");
        }
      }
    } else {
      error!(self.pos, "Unexpected EOF");
    }
  }

  fn dir_equ(&mut self, pos: Position) -> Directive {
    let name = if let Some(tok) = self.next_token() {
      match tok.var {
        TokenVar::Ident(s) => Self::to_string(&pos, s),
        _ => error!(tok.pos, "Expected identifier for equ directive"),
      }
    } else {
      error!(pos, "Unexpected EOF")
    };
    let constant = if let Some(mut tok) = self.next_token() {
      if let TokenVar::Comma = tok.var {
        tok = match self.next_token() {
          Some(t) => t,
          None => error!(pos, "Unexpected EOF"),
        };
      }
      let pos = tok.pos.clone();
      match Self::get_op_arg(tok) {
        Some(op) => op,
        None => error!(pos, "Unexpected newline"),
      }
    } else {
      error!(self.pos, "Unexpected EOF")
    };
    Directive {
      var: DirectiveVar::Const(name, constant, Public::Private),
      pos: pos,
    }
  }

  pub fn next_directive(&mut self) -> Option<Directive> {
    if let Some(tok) = self.next_token() {
      Some(match tok.var {
        TokenVar::Newline => return self.next_directive(),
        TokenVar::Label(label) => self.dir_label(tok.pos, label),
        TokenVar::Ident(op) => self.dir_ident(tok.pos, op),
        TokenVar::Data => self.dir_data(tok.pos),
        TokenVar::Public => self.dir_public(tok.pos),
        TokenVar::Import => self.dir_import(tok.pos),
        TokenVar::Equ => self.dir_equ(tok.pos),
        TokenVar::Macro | TokenVar::EndMacro =>
          error!(tok.pos, "Macros are not yet implemented"),
        tv => error!(tok.pos, "Unexpected {}", tv),
      })
    } else {
      None
    }
  }
}
