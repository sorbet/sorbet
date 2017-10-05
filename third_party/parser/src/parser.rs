use ast::*;
use ffi::Driver;
use std::rc::Rc;

pub struct ParserOptions<'a> {
    pub emit_file_vars_as_literals: bool,
    pub emit_lambda: bool,
    pub emit_procarg0: bool,
    pub declare_env: &'a [&'a str],
}

impl<'a> ParserOptions<'a> {
    fn defaults() -> Self {
        ParserOptions {
            emit_file_vars_as_literals: false,
            emit_lambda: true,
            emit_procarg0: true,
            declare_env: &[],
        }
    }
}

pub fn parse(source_file: Rc<SourceFile>) -> Ast {
    parse_with_opts(source_file, &ParserOptions::defaults())
}

pub fn parse_with_opts(source_file: Rc<SourceFile>, opts: &ParserOptions) -> Ast {
    let mut driver = Driver::new(source_file.clone());
    let ast = driver.parse(&opts);

    Ast {
        node: ast.map(|node| *node),
        diagnostics: driver.diagnostics(),
    }
}
