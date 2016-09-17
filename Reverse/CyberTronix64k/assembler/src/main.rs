use std::fs::File;
use std::io::Write;
use std::fmt::{self, Debug};

use clap::{Arg, App};

extern crate clap;
#[macro_use]
extern crate maplit;

#[macro_use]
mod macros;
mod lexer;
mod parser;

pub struct Opcode {
  var: OpcodeVariant,
  reg: u16,
  num: u16,
}

pub enum OpcodeVariant {
  MoveImmediate,
  Move,
  MoveDeref,
  Load,
  Store,
  Add,
  Sub,
  And,
  Or,
  Xor,
  ShiftRight,
  ShiftLeft,
  ShiftArithmetic,
  JumpGreater(u16),
  JumpLesser(u16),
  JumpEqual(u16),
  Data(Vec<u16>), // does not use reg or num
}

impl Debug for Opcode {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
    use OpcodeVariant::*;
    match self.var {
      MoveImmediate => write!(f, "MI {}, {}", self.reg, self.num),
      Move => write!(f, "MV {}, {}", self.reg, self.num),
      MoveDeref => write!(f, "MD {}, {}", self.reg, self.num),
      Load => write!(f, "LD {}, {}", self.reg, self.num),
      Store => write!(f, "ST {}, {}", self.reg, self.num),
      Add => write!(f, "AD {}, {}", self.reg, self.num),
      Sub => write!(f, "SB {}, {}", self.reg, self.num),
      And => write!(f, "ND {}, {}", self.reg, self.num),
      Or => write!(f, "OR {}, {}", self.reg, self.num),
      Xor => write!(f, "XR {}, {}", self.reg, self.num),
      ShiftRight => write!(f, "SR {}, {}", self.reg, self.num),
      ShiftLeft => write!(f, "SL {}, {}", self.reg, self.num),
      ShiftArithmetic => write!(f, "SA {}, {}", self.reg, self.num),
      JumpGreater(label) =>
        write!(f, "JG {}, {}, {}", self.reg, self.num, label),
      JumpLesser(label) =>
        write!(f, "JL {}, {}, {}", self.reg, self.num, label),
      JumpEqual(label) =>
        write!(f, "JQ {}, {}, {}", self.reg, self.num, label),
      Data(ref nums) => {
        try!(write!(f, "DATA "));
        for el in nums {
          try!(write!(f, "{} ", el));
        }
        write!(f, "ENDDATA")
      }
    }
  }
}

struct Program(Vec<Opcode>, parser::Parser);

impl Program {
  fn new(filename: &str) -> Self {
    let mut parser = parser::Parser::new(filename);
    let mut v = Vec::new();
    while let Some(el) = parser.next() {
      v.push(el);
    }
    Program(v, parser)
  }
}

fn main() {
  let matches =
    App::new("CT64k Assembler")
      .version("0.1")
      .author("Nicole Mazzuca <npmazzuca@gmail.com>")
      .about("A work in progress assembler for the CT64k")
      .arg(
        Arg::with_name("output")
          .short("o")
          .value_name("FILE")
          .help("Sets the output file to use")
          .required(true)
          .takes_value(true),
      ).arg(
        Arg::with_name("input")
          .help("Sets the input file to use")
          .required(true)
          .index(1),
      ).arg(
        Arg::with_name("print-labels")
          .short("p")
          .long("print-labels")
          .help("Sets whether the assembler prints the values of the labels")
      ).get_matches();

  let outfilename = matches.value_of("output").unwrap();
  let inpfilename = matches.value_of("input").unwrap();
  let print_labels = matches.is_present("print-labels");

  let program = Program::new(inpfilename);
  let mut out = Vec::new();
  for op in program.0 {
    use OpcodeVariant::*;

    fn write(out: &mut Vec<u8>, to_write: &[u16]) {
      let res = out.write_all(
        unsafe {
          std::slice::from_raw_parts(
            to_write.as_ptr() as *const u8,
            to_write.len() * 2,
          )
        }
      );
      match res {
        Ok(_) => {}
        Err(e) => {
          error_np!("Error while writing: {}", e);
        }
      }
    };
    fn arith(out: &mut Vec<u8>, opcode: u16, reg: u16, mem: u16) {
      write(out, &[(opcode << 12) | reg, mem]);
    };
    fn jump(out: &mut Vec<u8>, opcode: u16, reg: u16, mem: u16, label: u16) {
      write(out, &[(opcode << 12) | reg, mem, label]);
    };

    match op.var {
      MoveImmediate => arith(&mut out, 0x0, op.reg, op.num),
      Move => arith(&mut out, 0x1, op.reg, op.num),
      MoveDeref => arith(&mut out, 0x2, op.reg, op.num),
      Load => arith(&mut out, 0x3, op.reg, op.num),
      Store => arith(&mut out, 0x4, op.reg, op.num),
      Add => arith(&mut out, 0x5, op.reg, op.num),
      Sub => arith(&mut out, 0x6, op.reg, op.num),
      And => arith(&mut out, 0x7, op.reg, op.num),
      Or => arith(&mut out, 0x8, op.reg, op.num),
      Xor => arith(&mut out, 0x9, op.reg, op.num),
      ShiftRight => arith(&mut out, 0xA, op.reg, op.num),
      ShiftLeft => arith(&mut out, 0xB, op.reg, op.num),
      ShiftArithmetic => arith(&mut out, 0xC, op.reg, op.num),
      JumpGreater(label) => jump(&mut out, 0xD, op.reg, op.num, label),
      JumpLesser(label) => jump(&mut out, 0xE, op.reg, op.num, label),
      JumpEqual(label) => jump(&mut out, 0xF, op.reg, op.num, label),
      Data(nums) => write(&mut out, &nums),
    }
  }

  if print_labels {
    program.1.print_labels();
  }

  match File::create(outfilename) {
    Ok(mut file) => match file.write_all(&out) {
      Ok(_) => {}
      Err(e) => error_np!("Error while writing: {}", e),
    },
    Err(e) => error_np!(
      "Failed to open output file: `{}'\nError: {}", outfilename, e,
    ),
  };
}
