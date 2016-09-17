use std::collections::HashMap;
use std::path::{Path, PathBuf};
use {Opcode, OpcodeVariant};

use lexer::{
  self, Directive, DirectiveVar, Lexer, OpArg, OpArgVar, Position, Public,
};

const INST_OFFSET_BASE: u16 = 0x1000;

const REG_IP: u16 = 0x0;
const REG_SP: u16 = 0x1;
const REG_BP: u16 = 0x2;
const REG_SC0: u16 = 0x3;
const REG_SC1: u16 = 0x4;
const REG_SC2: u16 = 0x5;
const REG_SC3: u16 = 0x6;

#[derive(Copy, Clone)]
enum BaseOp {
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
  JumpGreater,
  JumpLesser,
  JumpEqual,
}

pub struct Parser {
  op_buffer: Vec<Opcode>,
  op_buffer_idx: usize,
  inst_offset: u16,
  directives: Vec<Directive>,
  labels: HashMap<String, u16>,
  macros: HashMap<String, (u16, Vec<(BaseOp, Vec<OpArg>)>)>,
  idx: usize,
}

impl Parser {
  pub fn new(filename: &str) -> Self {
    // compiler_defined_pos
    macro_rules! macro_op_arg {
      ($lexer:expr, $var:ident) => (
        OpArg {
          var: OpArgVar::$var,
          pos: $lexer.compiler_defined_pos(),
        }
      );
      ($lexer:expr, $var:ident ($($arg:tt)*)) => (
        OpArg {
          var: OpArgVar::$var($($arg)*),
          pos: $lexer.compiler_defined_pos(),
        }
      );
    }
    let path: PathBuf = match Path::new(filename).canonicalize() {
      Ok(c) => c,
      Err(_) => error_np!("Unable to open file: {}", filename),
    };
    let lexer = Lexer::new(&path);
    let mut this = Parser {
      op_buffer: Vec::new(),
      op_buffer_idx: 0,
      inst_offset: INST_OFFSET_BASE,
      directives: Vec::new(),
      labels: hashmap! {
        "ip".to_owned() => REG_IP,
        "sp".to_owned() => REG_SP,
        "bp".to_owned() => REG_BP,
        "sc0".to_owned() => REG_SC0,
        "sc1".to_owned() => REG_SC1,
        "sc2".to_owned() => REG_SC2,
        "sc3".to_owned() => REG_SC3,
      },
      macros: hashmap! {
        "mi".to_owned() => (2, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "mv".to_owned() => (2, vec![
          (BaseOp::Move, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1))
          ])
        ]),
        "md".to_owned() => (2, vec![
          (BaseOp::MoveDeref, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "ld".to_owned() => (2, vec![
          (BaseOp::Load, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "st".to_owned() => (2, vec![
          (BaseOp::Store, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "ad".to_owned() => (2, vec![
          (BaseOp::Add, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "sb".to_owned() => (2, vec![
          (BaseOp::Sub, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "nd".to_owned() => (2, vec![
          (BaseOp::And, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "or".to_owned() => (2, vec![
          (BaseOp::Or, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "xr".to_owned() => (2, vec![
          (BaseOp::Xor, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "sr".to_owned() => (2, vec![
          (BaseOp::ShiftRight, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "sl".to_owned() => (2, vec![
          (BaseOp::ShiftLeft, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "sa".to_owned() => (2, vec![
          (BaseOp::ShiftArithmetic, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ])
        ]),
        "jg".to_owned() => (3, vec![
          (BaseOp::JumpGreater, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
            macro_op_arg!(lexer, MacroArg(2)),
          ])
        ]),
        "jl".to_owned() => (3, vec![
          (BaseOp::JumpLesser, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
            macro_op_arg!(lexer, MacroArg(2)),
          ])
        ]),
        "jq".to_owned() => (3, vec![
          (BaseOp::JumpEqual, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, MacroArg(1)),
            macro_op_arg!(lexer, MacroArg(2)),
          ])
        ]),
        "hf".to_owned() => (0, vec![
          (BaseOp::JumpEqual, vec![
            macro_op_arg!(lexer, Number(REG_IP)),
            macro_op_arg!(lexer, Number(REG_IP)),
            macro_op_arg!(lexer, Here),
          ]),
        ]),
        "ji".to_owned() => (1, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_IP)),
            macro_op_arg!(lexer, MacroArg(0))
          ]),
        ]),
        "jm".to_owned() => (1, vec![
          (BaseOp::Move, vec![
            macro_op_arg!(lexer, Number(REG_IP)),
            macro_op_arg!(lexer, MacroArg(0)),
          ]),
        ]),
        "inc".to_owned() => (1, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, Number(1))
          ]),
          (BaseOp::Add, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
        ]),
        "dec".to_owned() => (1, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, Number(1)),
          ]),
          (BaseOp::Sub, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
        ]),
        "neg".to_owned() => (1, vec![
          (BaseOp::Move, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, MacroArg(0)),
          ]),
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(0)),
          ]),
          (BaseOp::Sub, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
        ]),
        "adi".to_owned() => (2, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ]),
          (BaseOp::Add, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
        ]),
        "sbi".to_owned() => (2, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, MacroArg(1)),
          ]),
          (BaseOp::Sub, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
        ]),
        "push".to_owned() => (1, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, Number(1)),
          ]),
          (BaseOp::Add, vec![
            macro_op_arg!(lexer, Number(REG_SP)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
          (BaseOp::Load, vec![
            macro_op_arg!(lexer, Number(REG_SP)),
            macro_op_arg!(lexer, MacroArg(0)),
          ]),
        ]),
        "pop".to_owned() => (1, vec![
          (BaseOp::MoveDeref, vec![
            macro_op_arg!(lexer, MacroArg(0)),
            macro_op_arg!(lexer, Number(REG_SP)),
          ]),
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, Number(1)),
          ]),
          (BaseOp::Sub, vec![
            macro_op_arg!(lexer, Number(REG_SP)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
        ]),
        "call".to_owned() => (1, vec![
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, Number(1)),
          ]),
          (BaseOp::Add, vec![
            macro_op_arg!(lexer, Number(REG_SP)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, ArithOp(
              lexer::ArithOp::Add,
              Box::new(macro_op_arg!(lexer, Here)),
              Box::new(macro_op_arg!(lexer, Number(6))),
            )),
          ]),
          (BaseOp::Load, vec![
            macro_op_arg!(lexer, Number(REG_SP)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_IP)),
            macro_op_arg!(lexer, MacroArg(0)),
          ])
        ]),
        "ret".to_owned() => (0, vec![
          (BaseOp::MoveDeref, vec![
            macro_op_arg!(lexer, Number(REG_SC1)),
            macro_op_arg!(lexer, Number(REG_SP)),
          ]),
          (BaseOp::MoveImmediate, vec![
            macro_op_arg!(lexer, Number(REG_SC0)),
            macro_op_arg!(lexer, Number(1)),
          ]),
          (BaseOp::Sub, vec![
            macro_op_arg!(lexer, Number(REG_SP)),
            macro_op_arg!(lexer, Number(REG_SC0)),
          ]),
          (BaseOp::Move, vec![
            macro_op_arg!(lexer, Number(REG_IP)),
            macro_op_arg!(lexer, Number(REG_SC1)),
          ])
        ]),
      },
      idx: 0,
    };

    this.get_directives(&mut vec![path], lexer);

    // normal labels
    let mut inst_offset = INST_OFFSET_BASE;
    for dir in &this.directives {
      match dir.var {
        DirectiveVar::Label(ref s, ref _public) => {
          // NOTE(ubsan): can optimize this to mem::replace(String::new())
          match this.labels.insert(s.clone(), inst_offset) {
            Some(_) => {
              error!(dir.pos, "Attempted to redefine label: {}", s);
            }
            None => {}
          }
        }
        DirectiveVar::Op(ref op, _) =>
          inst_offset += this.size_of_op_str(&dir.pos, op),
        DirectiveVar::Const(..) => {}
        DirectiveVar::Data(ref data) => inst_offset += data.len() as u16,
        DirectiveVar::Public(_) => {
          // TODO(ubsan): silently ignored for now
        },
        DirectiveVar::Import(_, _) => {},
        DirectiveVar::Macro{..} => unimplemented!(),
      }
    }

    // equ constants
    let mut inst_offset = INST_OFFSET_BASE;
    for dir in &this.directives {
      match dir.var {
        DirectiveVar::Label(..) => {}
        DirectiveVar::Op(ref op, _) =>
          inst_offset += this.size_of_op_str(&dir.pos, op),
        DirectiveVar::Const(ref s, ref arg, ref _public) => {
          let n = arg.evaluate(&this.labels, &[], inst_offset);
          match this.labels.insert(s.clone(), n) {
            Some(s) => error!(
              arg.pos, "Attempted to redefine label: {}", s
            ),
            None => {},
          }
        }
        DirectiveVar::Data(ref data) => inst_offset += data.len() as u16,
        DirectiveVar::Public(_) => {
          // TODO(ubsan): silently ignored for now
        },
        DirectiveVar::Import(_, _) => {},
        DirectiveVar::Macro{..} => unimplemented!(),
      }
    }

    this
  }

  pub fn print_labels(&self) {
    let mut labels = self.labels.iter().collect::<Vec<_>>();
    labels.sort_by(|a, b| <_ as ::std::cmp::Ord>::cmp(&a.1, &b.1));
    for (label, constant) in labels {
      if *constant >= INST_OFFSET_BASE {
        println!("{} = 0x{:X}", label, constant);
      }
    }
  }

  fn get_directives(&mut self, imports: &mut Vec<PathBuf>, mut lexer: Lexer) {
    use std::path::{Path, PathBuf};
    fn make_path(cur_path: &Path, pos: &Position, vec: Vec<String>) -> PathBuf {
      assert!(!vec.is_empty(), "ICE: DirectiveVar::Import had an empty Vec");
      let mut filename = PathBuf::new();
      for dir in &vec {
        filename.push(dir);
      }
      filename.set_extension("asm");

      let mut ret = PathBuf::from(cur_path);
      ret.set_file_name(filename);
      match ret.canonicalize() {
        Ok(c) => c,
        Err(_) => error!(pos, "failure to open import: {}", {
          let mut tmp = vec.iter().fold(String::new(), |mut s, el| {
            s.push_str(&el); s.push('.'); s
          });
          tmp.pop();
          tmp
        })
      }
    }
    fn push_unique(vec: &mut Vec<PathBuf>, to_push: PathBuf) -> bool {
      if vec.contains(&to_push) {
        false
      } else {
        vec.push(to_push);
        true
      }
    }
    let index = imports.len() - 1;
    while let Some(dir) = lexer.next_directive() {
      if let DirectiveVar::Import(path, _) = dir.var {
        let path = make_path(&imports[index], &dir.pos, path);
        if push_unique(imports, path) {
          let new_lexer = lexer.new_file_lexer(&imports[imports.len() - 1]);
          self.get_directives(imports, new_lexer);
        }
      } else {
        self.directives.push(dir);
      }
    }
  }

  fn size_of_op_str(&self, pos: &Position, op: &str) -> u16 {
    match self.macros.get(op) {
      Some(&(_, ref ops)) => {
        let mut acc = 0;
        for &(ref op, ref _args) in ops {
          acc += self.size_of_op(*op);
        }
        acc
      }
      None => error!(pos, "Unknown opcode: {}", op),
    }
  }

  fn size_of_op(&self, op: BaseOp) -> u16 {
    match op {
      BaseOp::MoveImmediate | BaseOp::Move | BaseOp::MoveDeref | BaseOp::Load
      | BaseOp::Store | BaseOp::Add | BaseOp::Sub | BaseOp::And | BaseOp::Or
      | BaseOp::Xor | BaseOp::ShiftRight | BaseOp::ShiftLeft
      | BaseOp::ShiftArithmetic => 2,
      BaseOp::JumpGreater | BaseOp::JumpLesser | BaseOp::JumpEqual => 3,
    }
  }

  fn next_directive(&mut self) -> Option<Directive> {
    if self.idx < self.directives.len() {
      let ret = ::std::mem::replace(&mut self.directives[self.idx], Directive {
        var: DirectiveVar::Label(String::new(), Public::Private),
        pos: Position::empty(),
      });
      self.idx += 1;
      Some(ret)
    } else {
      None
    }
  }
}

impl Iterator for Parser {
  type Item = Opcode;

  fn next(&mut self) -> Option<Opcode> {
    fn arith(
      this: &Parser,
      op: OpcodeVariant,
      args: &[OpArg],
      mac_args: &[OpArg],
    ) -> (Opcode, u16) {
      let reg = args[0].evaluate(&this.labels, mac_args, this.inst_offset);
      if reg >= 0x1000 {
        error!(
          mac_args[0].pos, "Register memory is out of range: {}", reg,
        );
      }
      let num = args[1].evaluate(&this.labels, mac_args, this.inst_offset);
      (Opcode {
        var: op,
        reg: reg,
        num: num,
      }, 2)
    }
    fn jump(
      this: &Parser,
      op: fn(u16) -> OpcodeVariant,
      args: &[OpArg],
      mac_args: &[OpArg],
    ) -> (Opcode, u16) {
      let reg = args[0].evaluate(&this.labels, mac_args, this.inst_offset);
      if reg >= 0x1000 {
        error!(
          mac_args[0].pos, "Register memory is out of range: {}", reg,
        );
      }
      let num = args[1].evaluate(&this.labels, mac_args, this.inst_offset);
      let label = args[2].evaluate(&this.labels, mac_args, this.inst_offset);
      (Opcode {
        var: op(label),
        reg: reg,
        num: num,
      }, 3)
    }
    fn data(this: &Parser, data: Vec<OpArg>) -> (Opcode, u16) {
      let mut data_num = Vec::new();
      // heh. datum.
      for datum in data {
        data_num.push(datum.evaluate(&this.labels, &[], this.inst_offset))
      }
      let offset = data_num.len() as u16;
      (Opcode {
        var: OpcodeVariant::Data(data_num),
        reg: 0,
        num: 0,
      }, offset)
    }
    // the u16 is the inst_offset to add
    fn opcode(
      this: &Parser, op: &BaseOp, args: &[OpArg], mac_args: &[OpArg],
    ) -> (Opcode, u16) {
      match *op {
        BaseOp::MoveImmediate =>
          arith(this, OpcodeVariant::MoveImmediate, args, &mac_args),
        BaseOp::Move => arith(this, OpcodeVariant::Move, args, &mac_args),
        BaseOp::MoveDeref =>
          arith(this, OpcodeVariant::MoveDeref, args, &mac_args),
        BaseOp::Load => arith(this, OpcodeVariant::Load, args, &mac_args),
        BaseOp::Store => arith(this, OpcodeVariant::Store, args, &mac_args),
        BaseOp::Add => arith(this, OpcodeVariant::Add, args, &mac_args),
        BaseOp::Sub => arith(this, OpcodeVariant::Sub, args, &mac_args),
        BaseOp::And => arith(this, OpcodeVariant::And, args, &mac_args),
        BaseOp::Or => arith(this, OpcodeVariant::Or, args, &mac_args),
        BaseOp::Xor => arith(this, OpcodeVariant::Xor, args, &mac_args),
        BaseOp::ShiftRight =>
          arith(this, OpcodeVariant::ShiftRight, args, &mac_args),
        BaseOp::ShiftLeft =>
          arith(this, OpcodeVariant::ShiftLeft, args, &mac_args),
        BaseOp::ShiftArithmetic =>
          arith(this, OpcodeVariant::ShiftArithmetic, args, &mac_args),
        BaseOp::JumpGreater =>
          jump(this, OpcodeVariant::JumpGreater, args, &mac_args),
        BaseOp::JumpLesser =>
          jump(this, OpcodeVariant::JumpLesser, args, &mac_args),
        BaseOp::JumpEqual =>
          jump(this, OpcodeVariant::JumpEqual, args, &mac_args),
      }
    }

    let op = match self.op_buffer.get_mut(self.op_buffer_idx) {
      Some(op) => Some(
        ::std::mem::replace(op, Opcode {
          var: OpcodeVariant::MoveImmediate,
          reg: 0,
          num: 0,
        })
      ),
      None => None,
    };
    if let Some(op) = op {
      self.op_buffer_idx += 1;
      return Some(op);
    } else {
      self.op_buffer_idx = 0;
      self.op_buffer.clear();
    }
    if let Some(dir) = self.next_directive() {
      match dir.var {
        DirectiveVar::Op(op, mac_args) => {
          match self.macros.get(&op) {
            Some(&(ref size, ref ops)) => {
              if (mac_args.len() as u16) != *size {
                error!(
                  dir.pos,
                  "Invalid number of args to {}; expected {}, found {}",
                  op,
                  size,
                  mac_args.len(),
                )
              }
              for &(ref op, ref args) in ops {
                let (op, offset) = opcode(self, op, args, &mac_args);
                self.inst_offset += offset;
                self.op_buffer.push(op);
              }
            },
            None => error!(dir.pos, "Unknown opcode"),
          }
          if let Some(op) = self.op_buffer.get_mut(0) {
            self.op_buffer_idx = 1;
            return Some(::std::mem::replace(op, Opcode {
              var: OpcodeVariant::MoveImmediate,
              reg: 0,
              num: 0,
            }));
          }
          self.next()
        },
        DirectiveVar::Data(nums) => {
          let (data, offset) = data(self, nums);
          self.inst_offset += offset;
          Some(data)
        }
        DirectiveVar::Label(..) | DirectiveVar::Const(..) => {
          while let Some(dir) = self.directives.get(self.idx) {
            match dir.var {
              DirectiveVar::Label(..) | DirectiveVar::Const(..) =>
                self.idx += 1,
              _ => break,
            }
          }
          self.next()
        },
        DirectiveVar::Public(_) => {
          // TODO(ubsan): silently ignored for now
          // there is no public|private distinction
          self.next()
        }
        DirectiveVar::Import(_, _) => {
          // imports aren't dealt with here
          self.next()
        },
        DirectiveVar::Macro{..} => unimplemented!(),
      }
    } else {
      None
    }
  }
}
