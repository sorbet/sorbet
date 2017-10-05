extern crate ruby_parser;
use ruby_parser::SourceFile;
use ruby_parser::parse;

use std::path::PathBuf;
use std::rc::Rc;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 2 {
        println!("Please pass in a file to highlight");
        return;
    }

    let path = PathBuf::from(&args[1]);
    let srcfile = SourceFile::open(path).unwrap();
    let ast = parse(Rc::new(srcfile));

    let mut out = String::new();
    let _ = ast.to_sexp(&mut out);
    println!("{}", out);
}
