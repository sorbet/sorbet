#![allow(non_camel_case_types, dead_code)]
use log::debug;
use std::ffi::CString;

#[derive(Clone, Copy, Debug, PartialEq)]
#[repr(transparent)]
pub struct VALUE(libc::uintptr_t);

#[derive(Clone, Copy)]
#[repr(transparent)]
pub struct ID(libc::uintptr_t);

#[repr(C)]
#[derive(Debug, Clone, Copy)]
pub enum ruby_value_type {
    RUBY_T_NONE = 0x00,

    RUBY_T_OBJECT = 0x01,
    RUBY_T_CLASS = 0x02,
    RUBY_T_MODULE = 0x03,
    RUBY_T_FLOAT = 0x04,
    RUBY_T_STRING = 0x05,
    RUBY_T_REGEXP = 0x06,
    RUBY_T_ARRAY = 0x07,
    RUBY_T_HASH = 0x08,
    RUBY_T_STRUCT = 0x09,
    RUBY_T_BIGNUM = 0x0a,
    RUBY_T_FILE = 0x0b,
    RUBY_T_DATA = 0x0c,
    RUBY_T_MATCH = 0x0d,
    RUBY_T_COMPLEX = 0x0e,
    RUBY_T_RATIONAL = 0x0f,

    RUBY_T_NIL = 0x11,
    RUBY_T_TRUE = 0x12,
    RUBY_T_FALSE = 0x13,
    RUBY_T_SYMBOL = 0x14,
    RUBY_T_FIXNUM = 0x15,
    RUBY_T_UNDEF = 0x16,

    RUBY_T_IMEMO = 0x1a,
    RUBY_T_NODE = 0x1b,
    RUBY_T_ICLASS = 0x1c,
    RUBY_T_ZOMBIE = 0x1d,

    RUBY_T_MASK = 0x1f,
}

#[allow(non_upper_case_globals)]
pub const Qnil: VALUE = VALUE(8);

extern "C" {
    // stuff that we need to compile out rubyfmt
    pub fn ruby_setup() -> libc::c_int;
    pub fn ruby_cleanup(_: libc::c_int);
    pub fn rb_eval_string_protect(_: *const libc::c_char, _: *mut libc::c_int) -> VALUE;
    pub fn rb_funcall(_: VALUE, _: ID, _: libc::c_int, ...) -> VALUE;
    pub fn rb_utf8_str_new(_: *const libc::c_char, _: libc::c_long) -> VALUE;
    pub fn rb_str_new_cstr(_: *const libc::c_char) -> VALUE;
    pub fn rb_string_value_cstr(_: *const VALUE) -> *const libc::c_char;
    pub fn rb_intern(_: *const libc::c_char) -> ID;
    pub fn rb_const_get_at(_: VALUE, _: ID) -> VALUE;
    pub fn Init_ripper();

    // Macros/inline functions wrapped as real functions
    pub fn rubyfmt_rstring_ptr(v: VALUE) -> *const libc::c_char;
    pub fn rubyfmt_rstring_len(v: VALUE) -> libc::c_long;
    pub fn rubyfmt_rb_type(v: VALUE) -> ruby_value_type;
    pub fn rubyfmt_rb_num2ll(v: VALUE) -> libc::c_longlong;
    pub fn rubyfmt_rb_ary_len(arr: VALUE) -> libc::c_long;
    pub fn rubyfmt_rb_ary_ptr(arr: VALUE) -> *mut VALUE;
    pub fn rubyfmt_rb_nil_p(arr: VALUE) -> libc::c_int;

    pub fn rb_protect(f: *const libc::c_void, arg: VALUE, state: *mut libc::c_int) -> VALUE;

    // C statics
    pub static rb_eRuntimeError: VALUE;
    pub static rb_mKernel: VALUE;
    pub static rb_cObject: VALUE;

    // C functions
    pub fn rb_sym2id(sym: VALUE) -> ID;
    pub fn rb_id2name(id: ID) -> *const libc::c_char;
    pub fn rb_ary_entry(arr: VALUE, idx: libc::c_long) -> VALUE;
    pub fn rb_raise(cls: VALUE, msg: *const libc::c_char);
    pub fn rb_block_call(
        obj: VALUE,
        method_id: ID,
        argc: libc::c_int,
        argv: *const VALUE,
        block: extern "C" fn(_: VALUE, _: VALUE, _: libc::c_int, _: *const VALUE) -> VALUE,
        outer_scope: VALUE,
    ) -> VALUE;
}

pub fn current_exception_as_rust_string() -> String {
    let ruby_string = unsafe { eval_str("$!.inspect") }.expect("Error evaluating `$!.inspect`");
    unsafe { ruby_string_to_str(ruby_string) }.to_owned()
}

macro_rules! intern {
    ($s:literal) => {
        rb_intern(concat!($s, "\0").as_ptr() as _)
    };
}

// Safety: This function expects an initialized Ruby VM capable of evaling code
pub unsafe fn eval_str(s: &str) -> Result<VALUE, ()> {
    let rubyfmt_program_as_c = CString::new(s).expect("unexpected nul byte in Ruby code");
    let mut state = 0;
    let v = rb_eval_string_protect(rubyfmt_program_as_c.as_ptr(), &mut state);
    if state != 0 {
        Err(())
    } else {
        Ok(v)
    }
}

extern "C" fn real_debug_inspect(v: VALUE) -> VALUE {
    unsafe {
        let inspect = rb_funcall(v, intern!("inspect"), 0);
        debug!("{}", ruby_string_to_str(inspect));
        Qnil
    }
}

pub fn debug_inspect(v: VALUE) {
    unsafe {
        let mut state = 0;
        rb_protect(real_debug_inspect as _, v, &mut state);
        if state != 0 {
            let s = current_exception_as_rust_string();
            panic!("blew us: {}", s);
        }
    }
}

pub fn raise(s: &str) {
    let cstr = CString::new(s).expect("input string has a null byte");
    unsafe {
        rb_raise(rb_eRuntimeError, cstr.as_ptr());
    }
}

// Safety: The given VALUE must be a valid Ruby array. The lifetime is not
// checked by the compiler. The returned slice must not outlive the given
// Ruby array. The Ruby array must not be modified while the returned slice
// is live.
pub unsafe fn ruby_array_to_slice<'a>(ary: VALUE) -> &'a [VALUE] {
    std::slice::from_raw_parts(rubyfmt_rb_ary_ptr(ary), rubyfmt_rb_ary_len(ary) as _)
}

// Safety: The given VALUE must be a valid Ruby string. The lifetime is not
// checked by the compiler. The returned str must not outlive the given
// Ruby string. The Ruby string must not be modified while the returned str
// is live.
pub unsafe fn ruby_string_to_str<'a>(s: VALUE) -> &'a str {
    let bytes =
        std::slice::from_raw_parts(rubyfmt_rstring_ptr(s) as _, rubyfmt_rstring_len(s) as _);
    std::str::from_utf8(bytes).expect("invalid UTF-8")
}
