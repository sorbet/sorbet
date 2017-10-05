extern crate ruby_parser;

#[macro_use]
mod helpers {
    #[macro_export]
    macro_rules! assert_sexp_eq {
        ($ours:expr , $theirs:expr, $filename:expr) => ({
            let ours = $ours;
            let theirs = $theirs;
            if ours.len() != theirs.len() {
                let ch = Changeset::new(ours, theirs, "\n");
                if ch.distance != 0 {
                    println!("Mismatch in '{}':", $filename);
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
        })
    }

    #[macro_export]
    macro_rules! assert_diag {
        ($code:expr , $expect_level:expr , $expect_error:expr , $opts:expr) => ({
            let code = $code;
            let src = Rc::new(ruby_parser::SourceFile::new(
                    PathBuf::from("(assert_diagnostics)"), code.to_owned()));
            let ast = ruby_parser::parse_with_opts(src, &$opts);
            assert_eq!(ast.diagnostics.len(), 1);

            let err = ast.diagnostics.first().unwrap();
            assert_eq!(err.level, $expect_level);
            assert_eq!(err.error, $expect_error);
        })
    }

    #[macro_export]
    macro_rules! parse_and_cmp {
        ($code:expr , $sexp:expr , $opts:expr) => ({
            let code = $code;
            let sexp = $sexp.trim();
            let src = Rc::new(ruby_parser::SourceFile::new(
                    PathBuf::from("(assert_parses)"), code.to_owned()));
            let ast = ruby_parser::parse_with_opts(src, &$opts);

            let mut buf = String::new();
            ast.to_sexp(&mut buf).expect("failed to write sexp output");
            assert_sexp_eq!(buf.as_str(), sexp, "(test case)");
        })
    }
}
