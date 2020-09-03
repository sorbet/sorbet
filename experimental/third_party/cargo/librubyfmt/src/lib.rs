#![deny(warnings, missing_copy_implementations)]

use serde::de::value;
use std::io::{Cursor, Write};
use std::slice;
use std::str;

pub type RawStatus = i64;

#[macro_use]
mod ruby;
mod comment_block;
mod de;
mod delimiters;
mod file_comments;
mod format;
mod intermediary;
mod line_metadata;
mod line_tokens;
mod parser_state;
mod render_queue_writer;
mod render_targets;
mod ripper_tree_types;
mod ruby_ops;
mod types;

use file_comments::FileComments;
use parser_state::ParserState;
use ruby_ops::{load_rubyfmt, ParseError, Parser, RipperTree};

#[cfg(debug_assertions)]
use log::debug;
#[cfg(debug_assertions)]
use simplelog::{Config, LevelFilter, TermLogger, TerminalMode};

extern "C" {
    pub fn Init_ripper();
}

pub struct RubyfmtString(Box<str>);

#[derive(Debug, Copy, Clone)]
pub enum InitStatus {
    OK = 0,
    ERROR = 1,
}

#[derive(Debug)]
pub enum RichFormatError {
    SyntaxError,
    RipperParseFailure(value::Error),
    IOError(std::io::Error),
    OtherRubyError(String),
}

impl RichFormatError {
    fn into_format_error(self) -> FormatError {
        match self {
            RichFormatError::SyntaxError => FormatError::SyntaxError,
            RichFormatError::RipperParseFailure(_) => FormatError::RipperParseFailure,
            RichFormatError::IOError(_) => FormatError::IOError,
            RichFormatError::OtherRubyError(_) => FormatError::OtherRubyError,
        }
    }
}

#[derive(Debug, Copy, Clone)]
pub enum FormatError {
    OK = 0,
    SyntaxError = 1,
    RipperParseFailure = 2,
    IOError = 3,
    OtherRubyError = 4,
}

pub fn format_buffer(buf: &str) -> Result<String, RichFormatError> {
    let (tree, file_comments) = run_parser_on(buf)?;
    let out_data = vec![];
    let mut output = Cursor::new(out_data);
    toplevel_format_program(&mut output, tree, file_comments)?;
    output.flush().expect("flushing to a vec should never fail");
    Ok(String::from_utf8(output.into_inner()).expect("we never write invalid UTF-8"))
}

#[no_mangle]
pub extern "C" fn rubyfmt_init() -> libc::c_int {
    init_logger();
    let res = ruby_ops::setup_ruby();
    if res.is_err() {
        return InitStatus::ERROR as libc::c_int;
    }

    let res = unsafe { load_ripper() };
    if res.is_err() {
        return InitStatus::ERROR as libc::c_int;
    }

    let res = unsafe { load_rubyfmt() };
    if res.is_err() {
        return InitStatus::ERROR as libc::c_int;
    }

    InitStatus::OK as libc::c_int
}

/// # Safety
/// this function will fail, very badly, if len specifies more bytes than is
/// available in the passed buffer pointer. It will also fail if the passed
/// data isn't utf8.
/// Please don't pass non-utf8 too small buffers.
#[no_mangle]
pub unsafe extern "C" fn rubyfmt_format_buffer(
    ptr: *const u8,
    len: usize,
    err: *mut i64,
) -> *mut RubyfmtString {
    let input = str::from_utf8_unchecked(slice::from_raw_parts(ptr, len));
    let output = format_buffer(input);
    match output {
        Ok(o) => {
            *err = FormatError::OK as i64;
            Box::into_raw(Box::new(RubyfmtString(o.into_boxed_str())))
        }
        Err(e) => {
            *err = e.into_format_error() as i64;
            std::ptr::null::<RubyfmtString>() as _
        }
    }
}

#[no_mangle]
pub extern "C" fn rubyfmt_string_ptr(s: &RubyfmtString) -> *const u8 {
    s.0.as_ptr()
}

#[no_mangle]
pub extern "C" fn rubyfmt_string_len(s: &RubyfmtString) -> usize {
    s.0.len()
}

#[no_mangle]
extern "C" fn rubyfmt_string_free(rubyfmt_string: *mut RubyfmtString) {
    unsafe {
        Box::from_raw(rubyfmt_string);
    }
}

// Safety: This function expects a functioning Ruby VM
unsafe fn load_ripper() -> Result<(), ()> {
    // trick ruby in to thinking ripper is already loaded
    ruby::eval_str(
        r#"
    $LOADED_FEATURES << "ripper.bundle"
    $LOADED_FEATURES << "ripper.so"
    $LOADED_FEATURES << "ripper.rb"
    $LOADED_FEATURES << "ripper/core.rb"
    $LOADED_FEATURES << "ripper/sexp.rb"
    $LOADED_FEATURES << "ripper/filter.rb"
    $LOADED_FEATURES << "ripper/lexer.rb"
    "#,
    )?;

    // init the ripper C module
    Init_ripper();

    //load each ripper program
    ruby::eval_str(include_str!(
        "../ruby_checkout/ruby-2.6.6/ext/ripper/lib/ripper.rb"
    ))?;
    ruby::eval_str(include_str!(
        "../ruby_checkout/ruby-2.6.6/ext/ripper/lib/ripper/core.rb"
    ))?;
    ruby::eval_str(include_str!(
        "../ruby_checkout/ruby-2.6.6/ext/ripper/lib/ripper/lexer.rb"
    ))?;
    ruby::eval_str(include_str!(
        "../ruby_checkout/ruby-2.6.6/ext/ripper/lib/ripper/filter.rb"
    ))?;
    ruby::eval_str(include_str!(
        "../ruby_checkout/ruby-2.6.6/ext/ripper/lib/ripper/sexp.rb"
    ))?;

    Ok(())
}

pub fn toplevel_format_program<W: Write>(
    writer: &mut W,
    tree: RipperTree,
    file_comments: FileComments,
) -> Result<(), RichFormatError> {
    let mut ps = ParserState::new(file_comments);
    let v: ripper_tree_types::Program =
        de::from_value(tree).map_err(RichFormatError::RipperParseFailure)?;

    format::format_program(&mut ps, v);

    ps.write(writer).map_err(RichFormatError::IOError)?;
    writer.flush().map_err(RichFormatError::IOError)?;
    Ok(())
}

fn run_parser_on(buf: &str) -> Result<(RipperTree, FileComments), RichFormatError> {
    Parser::new(buf).parse().map_err(|e| match e {
        ParseError::SyntaxError => RichFormatError::SyntaxError,
        ParseError::OtherRubyError(s) => RichFormatError::OtherRubyError(s),
    })
}

fn init_logger() {
    #[cfg(debug_assertions)]
    {
        TermLogger::init(LevelFilter::Debug, Config::default(), TerminalMode::Stderr)
            .expect("making a term logger");
        debug!("logger works");
    }
}
