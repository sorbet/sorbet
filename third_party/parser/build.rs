extern crate gcc;

use std::fs;
use std::process::Command;

fn main() {
    if fs::metadata("Makefile").is_ok() {
        Command::new("make").status().unwrap();
    }

    gcc::Config::new()
		.cpp(true)
		.flag("-std=c++14")
		.flag("-Wall")
		.flag("-Wextra")
		.flag("-Wpedantic")
		.flag("-Wno-unused-const-variable")
		.include("include")
		.file("cc/capi.cc")
		.file("cc/lexer.cc")
		.file("cc/literal.cc")
		.file("cc/driver.cc")
		.file("cc/state_stack.cc")
		.file("cc/token.cc")
		.file("cc/grammars/typedruby24.cc")
		.compile("librubyparser.a")
}
