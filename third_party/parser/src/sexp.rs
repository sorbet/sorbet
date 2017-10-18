use std::ops::Deref;
use std::fmt;
use std::rc::Rc;
use ast::*;

pub trait Sexp {
    fn sexp(&self, f: &mut SexpFormatter) -> fmt::Result;
}

pub struct SexpFormatter<'a> {
    indent: usize,
    buf: &'a mut (fmt::Write+'a),
    print_loc: bool,
    print_str: bool,
}

impl<'a> SexpFormatter<'a> {
    pub fn write_str(&mut self, data: &str) -> fmt::Result {
        self.buf.write_str(data)
    }

    pub fn write_fmt(&mut self, fmt: fmt::Arguments) -> fmt::Result {
        fmt::write(self.buf, fmt)
    }

    #[inline]
    pub fn new_node<'b>(&'b mut self, name: &str) -> SexpNode<'b, 'a> {
        sexp_node_new(self, name)
    }
}

pub struct SexpNode<'a, 'b: 'a> {
    fmt: &'a mut SexpFormatter<'b>,
    result: fmt::Result,
}

pub fn sexp_node_new<'a, 'b>(fmt: &'a mut SexpFormatter<'b>, name: &str) -> SexpNode<'a, 'b> {
    let indent = fmt.indent*2;
    let result = write!(fmt, "{:width$}({}", "", name, width=indent);
    SexpNode {
        fmt: fmt,
        result: result,
    }
}

fn escape_rb(f: &mut SexpFormatter, s: &str) -> fmt::Result {
    f.buf.write_char('"')?;
    let mut from = 0;
    for (i, c) in s.char_indices() {
        let esc = c.escape_default();
        if esc.len() != 1 {
            f.write_str(&s[from..i])?;
            for c in esc {
                f.buf.write_char(c)?;
            }
            from = i + c.len_utf8();
        }
    }
    f.buf.write_str(&s[from..])?;
    f.buf.write_char('"')
}

impl<'a, 'b: 'a> SexpNode<'a, 'b> {
    pub fn field(&mut self, value: &Sexp) -> &mut SexpNode<'a, 'b> {
        self.field_with(|fmt| value.sexp(fmt))
    }

    pub fn field_with<F>(&mut self, f: F) -> &mut SexpNode<'a, 'b>
        where F: FnOnce(&mut SexpFormatter) -> fmt::Result
    {
        self.result = self.result.and_then(|_| {
            self.fmt.indent += 1;
            let res = f(self.fmt);
            self.fmt.indent -= 1;
            res
        });
        self
    }

    pub fn string(&mut self, value: &RubyString) -> &mut SexpNode<'a, 'b> {
        self.result = self.result.and_then(|_| {
            if self.fmt.print_str {
                self.fmt.buf.write_char(' ')?;
                match value.string() {
                    Some(ref string) => escape_rb(self.fmt, string),
                    None => write!(self.fmt, "[BINARY STRING]"),
                }
            } else {
                write!(self.fmt, " [STRING]")
            }
        });
        self
    }

    pub fn numeric(&mut self, value: &String) -> &mut SexpNode<'a, 'b> {
        self.result = self.result.and_then(|_| {
            write!(self.fmt, " {}", value.replace("_", ""))
        });
        self
    }

    pub fn finish(&mut self) -> fmt::Result {
        self.result.and_then(|_| {
            self.fmt.write_str(")")
        })
    }
}

impl Sexp for Vec<Rc<Node>> {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        for ref n in self.iter() {
            n.sexp(w)?;
        }
        Ok(())
    }
}

impl Sexp for Loc {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        if w.print_loc {
            write!(w, " @{:?}", self)
        } else {
            Ok(())
        }
    }
}

impl Sexp for Rc<Node> {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        if w.indent > 0 {
            write!(w, "\n")?;
        }
        self.deref().sexp(w)
    }
}

impl Sexp for Option<Rc<Node>> {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        match self {
            &Some(ref n) => n.sexp(w),
            &None => write!(w, " nil"),
        }
    }
}

impl Sexp for Id {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        match self {
            &Id(_, ref id) => id.sexp(w)
        }
    }
}

fn sym_print_p(sym: &str) -> bool {
    if sym.len() == 1 {
        return true;
    }

    if sym.chars().any(|c| c.is_whitespace()) {
        return false;
    }

    if sym.chars().all(|c| !c.is_alphanumeric()) {
        return true;
    }

    !sym.chars().any(|c| { c == '.' || c == '-' })
}

impl Sexp for String {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        if sym_print_p(self.as_str()) {
            write!(w, " :{}", self)
        } else {
            write!(w, " :")?;
            escape_rb(w, self.as_str())
        }
    }
}

impl Sexp for Vec<char> {
    fn sexp(&self, f: &mut SexpFormatter) -> fmt::Result {
        if !self.is_empty() {
            for b in self.iter() {
                write!(f, " :{}", b)?;
            }
        }
        Ok(())
    }
}

impl Sexp for usize {
    fn sexp(&self, f: &mut SexpFormatter) -> fmt::Result {
        write!(f, " {}", self)
    }
}

impl Sexp for Option<Id> {
    fn sexp(&self, w: &mut SexpFormatter) -> fmt::Result {
        match self {
            &Some(ref n) => { n.sexp(w) },
            &None => Ok(())
        }
    }
}

fn args_sexp(node: Option<Rc<Node>>, w: &mut SexpFormatter) -> fmt::Result {
    match node {
        None => {
            if w.indent > 0 {
                write!(w, "\n")?;
            }
            w.new_node("args").finish()
        }
        Some(node) => node.sexp(w),
    }
}

impl Sexp for Node {
    fn sexp(&self, __arg_0: &mut SexpFormatter) -> fmt::Result {
        match (&*self,) {
            (&Node::Alias(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("alias");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::And(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("and");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::AndAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("and-asgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Arg(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("arg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Args(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("args");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Array(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("array");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Backref(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("back-ref");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Begin(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("begin");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Block(ref loc, ref send, ref block_args,
                          ref block_body),) => {
                let mut builder = __arg_0.new_node("block");
                let _ = builder.field(loc);
                let _ = builder.field(send);
                let _ = builder.field_with(|fmt| args_sexp(block_args.clone(), fmt));
                let _ = builder.field(block_body);
                builder.finish()
            }
            (&Node::Blockarg(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("blockarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::BlockPass(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("block-pass");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Break(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("break");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Case(ref __self_0, ref __self_1, ref __self_2,
                         ref __self_3),) => {
                let mut builder = __arg_0.new_node("case");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Cbase(ref __self_0),) => {
                let mut builder = __arg_0.new_node("cbase");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::Class(ref __self_0, ref __self_1, ref __self_2,
                          ref __self_3),) => {
                let mut builder = __arg_0.new_node("class");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Complex(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("complex");
                let _ = builder.field(__self_0);
                let _ = builder.numeric(__self_1);
                builder.finish()
            }
            (&Node::Const(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("const");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::ConstAsgn(ref __self_0, ref __self_1, ref __self_2,
                          ref __self_3),) => {
                let mut builder = __arg_0.new_node("casgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::ConstLhs(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("casgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::CSend(ref __self_0, ref __self_1, ref __self_2,
                          ref __self_3),) => {
                let mut builder = __arg_0.new_node("csend");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Cvar(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("cvar");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::CvarAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("cvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::CvarLhs(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("cvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Def(ref loc, ref id, ref args, ref body),) => {
                let mut builder = __arg_0.new_node("def");
                let _ = builder.field(loc);
                let _ = builder.field(id);
                let _ = builder.field_with(|fmt| args_sexp(args.clone(), fmt));
                let _ = builder.field(body);
                builder.finish()
            }
            (&Node::Defined(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("defined?");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Defs(ref loc, ref definee, ref id,
                         ref args, ref body),) => {
                let mut builder = __arg_0.new_node("defs");
                let _ = builder.field(loc);
                let _ = builder.field(definee);
                let _ = builder.field(id);
                let _ = builder.field_with(|fmt| args_sexp(args.clone(), fmt));
                let _ = builder.field(body);
                builder.finish()
            }
            (&Node::DString(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("dstr");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::DSymbol(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("dsym");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::EFlipflop(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("eflipflop");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::EncodingLiteral(ref __self_0),) => {
                let mut builder = __arg_0.new_node("--ENCODING--");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::Ensure(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ensure");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::ERange(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("erange");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::False(ref __self_0),) => {
                let mut builder = __arg_0.new_node("false");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::FileLiteral(ref __self_0),) => {
                let mut builder = __arg_0.new_node("--FILE--");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::For(ref __self_0, ref __self_1, ref __self_2,
                        ref __self_3),) => {
                let mut builder = __arg_0.new_node("for");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Float(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("float");
                let _ = builder.field(__self_0);
                let _ = builder.numeric(__self_1);
                builder.finish()
            }
            (&Node::Gvar(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("gvar");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::GvarAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("gvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::GvarLhs(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("gvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Hash(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("hash");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Ident(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ident");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::If(ref __self_0, ref __self_1, ref __self_2,
                       ref __self_3),) => {
                let mut builder = __arg_0.new_node("if");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::IFlipflop(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("iflipflop");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Integer(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("int");
                let _ = builder.field(__self_0);
                let _ = builder.numeric(__self_1);
                builder.finish()
            }
            (&Node::IRange(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("irange");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Ivar(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ivar");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::IvarAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ivasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::IvarLhs(ref __self_0, ref __self_1),) => {
                // intentionally rendering Ivlhs as an Ivasgn for AST compatibility
                // with parser gem:
                let mut builder = __arg_0.new_node("ivasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Kwarg(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("kwarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Kwbegin(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("kwbegin");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Kwoptarg(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("kwoptarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Kwrestarg(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("kwrestarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Kwsplat(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("kwsplat");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Lambda(ref __self_0),) => {
                let mut builder = __arg_0.new_node("lambda");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::LineLiteral(ref __self_0),) => {
                let mut builder = __arg_0.new_node("lineLiteral");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::Lvar(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("lvar");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::LvarAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("lvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::LvarLhs(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("lvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::MatchAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("match-with-lvasgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::MatchCurLine(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("match-current-line");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Masgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("masgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Mlhs(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("mlhs");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Module(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("module");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Next(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("next");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Nil(ref __self_0),) => {
                let mut builder = __arg_0.new_node("nil");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::NthRef(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("nth-ref");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::OpAsgn(ref __self_0, ref __self_1, ref __self_2,
                           ref __self_3),) => {
                let mut builder = __arg_0.new_node("op-asgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Optarg(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("optarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Or(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("or");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::OrAsgn(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("or-asgn");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Pair(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("pair");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Postexe(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("postexe");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Preexe(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("preexe");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Procarg0(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("procarg0");
                let _ = builder.field(__self_0);
                let _ = match **__self_1 {
                    Node::Arg(_, ref arg) => builder.field(arg),
                    _ => builder.field(__self_1),
                };
                builder.finish()
            }
            (&Node::Rational(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("rational");
                let _ = builder.field(__self_0);
                let _ = builder.numeric(__self_1);
                builder.finish()
            }
            (&Node::Redo(ref __self_0),) => {
                let mut builder = __arg_0.new_node("redo");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::Regexp(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("regexp");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Regopt(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("regopt");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Resbody(ref __self_0, ref __self_1, ref __self_2,
                            ref __self_3),) => {
                let mut builder = __arg_0.new_node("resbody");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Rescue(ref __self_0, ref __self_1, ref __self_2,
                           ref __self_3),) => {
                let mut builder = __arg_0.new_node("rescue");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::Restarg(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("restarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Retry(ref __self_0),) => {
                let mut builder = __arg_0.new_node("retry");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::Return(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("return");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::SClass(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("sclass");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Self_(ref __self_0),) => {
                let mut builder = __arg_0.new_node("self");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::Send(ref __self_0, ref __self_1, ref __self_2,
                         ref __self_3),) => {
                let mut builder = __arg_0.new_node("send");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::ShadowArg(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("shadowarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Splat(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("splat");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::SplatLhs(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("splat");
                let _ = builder.field(__self_0);
                if __self_1.is_some() {
                    let _ = builder.field(__self_1);
                }
                builder.finish()
            }
            (&Node::String(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("str");
                let _ = builder.field(__self_0);
                let _ = builder.string(__self_1);
                builder.finish()
            }
            (&Node::Super(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("super");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Symbol(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("sym");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::True(ref __self_0),) => {
                let mut builder = __arg_0.new_node("true");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::TyAny(ref __self_0),) => {
                let mut builder = __arg_0.new_node("ty-any");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::TyArray(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-array");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TyCast(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-cast");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyClass(ref __self_0),) => {
                let mut builder = __arg_0.new_node("ty-class");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::TyConstInstance(ref loc, ref cpath, ref params),) => {
                let mut builder = __arg_0.new_node("ty-const-instance");
                let _ = builder.field(loc);
                let _ = builder.field(cpath);
                let _ = builder.field(params);
                builder.finish()
            }
            (&Node::TyConSubtype(ref loc, ref sub, ref super_),) => {
                let mut builder = __arg_0.new_node("ty-consubtype");
                let _ = builder.field(loc);
                let _ = builder.field(sub);
                let _ = builder.field(super_);
                builder.finish()
            }
            (&Node::TyConUnify(ref loc, ref a, ref b),) => {
                let mut builder = __arg_0.new_node("ty-conunify");
                let _ = builder.field(loc);
                let _ = builder.field(a);
                let _ = builder.field(b);
                builder.finish()
            }
            (&Node::TyCpath(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-cpath");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TyGenargs(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-genargs");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyGendecl(ref __self_0, ref __self_1, ref __self_2, ref __self_3),) => {
                let mut builder = __arg_0.new_node("ty-gendecl");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::TyGendeclarg(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-gendeclarg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyGeninst(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-geninst");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyHash(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-hash");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyInstance(ref __self_0),) => {
                let mut builder = __arg_0.new_node("ty-instance");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::TyIvardecl(ref __self_0, ref __self_1, ref __self_2),) =>
            {
                let mut builder = __arg_0.new_node("ty-ivardecl");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyNil(ref __self_0),) => {
                let mut builder = __arg_0.new_node("ty-nil");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::TyNillable(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-nillable");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TyOr(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-or");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::TyParen(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-paren");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TyProc(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-proc");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TyPrototype(ref __self_0, ref __self_1, ref __self_2,
                              ref __self_3),) => {
                let mut builder = __arg_0.new_node("ty-prototype");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                let _ = builder.field(__self_3);
                builder.finish()
            }
            (&Node::TyReturnSig(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-return-sig");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TySelf(ref __self_0),) => {
                let mut builder = __arg_0.new_node("ty-self");
                let _ = builder.field(__self_0);
                builder.finish()
            }
            (&Node::TyTuple(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("ty-tuple");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::TyTypedArg(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("ty-typed-arg");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::Undef(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("undef");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Until(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("until");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::UntilPost(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("until-post");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::When(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("when");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::While(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("while");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::WhilePost(ref __self_0, ref __self_1, ref __self_2),) => {
                let mut builder = __arg_0.new_node("while-post");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                let _ = builder.field(__self_2);
                builder.finish()
            }
            (&Node::XString(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("xstr");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::Yield(ref __self_0, ref __self_1),) => {
                let mut builder = __arg_0.new_node("yield");
                let _ = builder.field(__self_0);
                let _ = builder.field(__self_1);
                builder.finish()
            }
            (&Node::ZSuper(ref __self_0),) => {
                let mut builder = __arg_0.new_node("zsuper");
                let _ = builder.field(__self_0);
                builder.finish()
            }
        }
    }
}

impl Node {
    pub fn debug_ast(&self) -> String {
        let mut output = String::new();

        let result = {
            let mut formatter = SexpFormatter {
                indent: 0,
                buf: &mut output,
                print_loc: false,
                print_str: true,
            };
            self.sexp(&mut formatter)
        };

        result.unwrap();
        output
    }
}

impl Ast {
    pub fn to_sexp(&self, output: &mut fmt::Write) -> fmt::Result {
        match self.node {
            Some(ref node) => {
                let mut formatter = SexpFormatter {
                    indent: 0,
                    buf: output,
                    print_loc: false,
                    print_str: true,
                };
                node.sexp(&mut formatter)
            }
            None => Ok(())
        }
    }
}
