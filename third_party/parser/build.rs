extern crate cc;

use std::env;
use std::process::Command;

fn main() {
    let out_dir = env::var("OUT_DIR").unwrap();

    let lexer_cc = &format!("{}/lexer.cc", out_dir);
    Command::new("ragel")
        .args(&["-o", lexer_cc, "-C", "cc/lexer.rl"])
        .status().unwrap();

    let grammar_cc = &format!("{}/typedruby24.cc", out_dir);
    Command::new("bison")
        .args(&[
            &format!("--defines={}/typedruby24.hh", out_dir),
            "-o", grammar_cc, "cc/grammars/typedruby24.ypp"])
        .status().unwrap();

    Command::new("ruby")
        .args(&["codegen/builder.rb",
            "--rs", &format!("{}/ffi_builder.rs", out_dir)])
        .status().unwrap();

    Command::new("ruby")
        .args(&["codegen/diagnostics.rb",
            "--rs", &format!("{}/ffi_diagnostics.rs", out_dir),
            "--cpp", &format!("{}/diagnostic_class.hh", out_dir)])
        .status().unwrap();

    cc::Build::new()
        .cpp(true)
        .flag("-std=c++14")
        .flag("-Wall")
        .flag("-Wextra")
        .flag("-Wpedantic")
        .flag("-Wno-unused-const-variable")
        .include("include")
        .include(out_dir)
        .file(lexer_cc)
        .file(grammar_cc)
        .file("cc/capi.cc")
        .file("cc/literal.cc")
        .file("cc/driver.cc")
        .file("cc/state_stack.cc")
        .file("cc/token.cc")
        .compile("librubyparser.a")
}
