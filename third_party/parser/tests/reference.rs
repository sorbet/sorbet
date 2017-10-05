extern crate difference;
extern crate glob;
extern crate ruby_parser;

use glob::glob;
use std::rc::Rc;
use std::fs::File;
use std::io::Read;
use std::path::{Path, PathBuf};
use difference::{Changeset, Difference};

fn known_failure(path: &Path) -> bool {
    match path.to_str().unwrap() {
        // Mlhs
        "library/32/ae1517d8058d13e7948d890377bf77.rb" => true,
        "library/bf/48d40cd135bd8083711e089549b373.rb" => true,

        // match-with-lvasgn
        "library/a4/706545d5c4055aec5b11c904aeaa79.rb" => true,

        // Anything else should be green
        _ => false,
    }
}

fn compare_sexps(path: PathBuf) {
    let mut buf_rs = String::new();
    let mut buf_rb = String::new();

    let sexp_path = path.with_extension("rbast");
    let src = Rc::new(ruby_parser::SourceFile::open(path).expect("failed to load file"));
    let ast = ruby_parser::parse(src.clone());

    {
        let mut file = File::open(&sexp_path).unwrap();
        file.read_to_string(&mut buf_rb).expect("failed to read sexp");
        assert!(buf_rb.len() > 0, "empty file at {}", sexp_path.display());
    }

    ast.to_sexp(&mut buf_rs).expect("failed to write sexp output");
    assert!(buf_rs.len() > 0);

    if buf_rs.len() != buf_rb.len() {
        let ch = Changeset::new(buf_rs.as_str(), buf_rb.as_str(), "\n");
        if ch.distance != 0 {
            println!("Mismatch in '{}':", src.filename().display());
            for d in &ch.diffs {
                match *d {
                    Difference::Add(ref x) => {
                        println!("\x1b[92m{}\x1b[0m", x);
                    }
                    Difference::Rem(ref x) => {
                        println!("\x1b[91m{}\x1b[0m", x);
                    }
                    Difference::Same(_) => {}
                }
            }
        }
        assert_eq!(ch.distance, 0);
    }
}

#[test]
#[ignore]
fn compare_full_library() {
    for entry in glob("library/**/*.rb").unwrap() {
        let path = entry.expect("failed to glob path");
        if !known_failure(&path) {
            compare_sexps(path);
        }
    }
}

#[test]
fn compare_sinatra() {
    let path = PathBuf::from("tests/fixtures/sinatra.rb");
    compare_sexps(path);
}
