use crate::ruby::{self, VALUE};
use crate::ruby_ops::RipperTree;
use serde::de::{self, Error as _};
use std::mem::size_of;

pub fn from_value<T: de::DeserializeOwned>(v: RipperTree) -> Result<T> {
    T::deserialize(Deserializer(v.into_value()))
}

#[derive(Clone, Copy)]
struct Deserializer(VALUE);

type Result<T> = std::result::Result<T, de::value::Error>;
type Error = de::value::Error;

impl<'de> serde::Deserializer<'de> for Deserializer {
    type Error = Error;

    fn deserialize_any<V: de::Visitor<'de>>(self, visitor: V) -> Result<V::Value> {
        pub use ruby::ruby_value_type::*;

        match unsafe { ruby::rubyfmt_rb_type(self.0) } {
            RUBY_T_SYMBOL => visitor.visit_borrowed_str(sym_to_str(self.0)?),
            RUBY_T_STRING => visitor.visit_borrowed_str(rstring_to_str(self.0)?),
            RUBY_T_ARRAY => visitor.visit_seq(SeqAccess::new(self.0)),
            RUBY_T_NIL => visitor.visit_none(),
            RUBY_T_TRUE => visitor.visit_bool(true),
            RUBY_T_FALSE => visitor.visit_bool(false),
            RUBY_T_FIXNUM => visitor.visit_i64(unsafe { ruby::rubyfmt_rb_num2ll(self.0) }),
            other => Err(de::Error::custom(format_args!(
                "Unexpected type {:?}",
                other
            ))),
        }
    }

    fn deserialize_option<V: de::Visitor<'de>>(self, visitor: V) -> Result<V::Value> {
        if unsafe { ruby::rubyfmt_rb_nil_p(self.0) != 0 } {
            visitor.visit_none()
        } else {
            visitor.visit_some(self)
        }
    }

    fn deserialize_newtype_struct<V: de::Visitor<'de>>(
        self,
        name: &str,
        visitor: V,
    ) -> Result<V::Value> {
        if name == "VALUE" {
            let bytes: &[u8; size_of::<VALUE>()] =
                unsafe { &*(&self.0 as *const VALUE as *const _) };
            visitor.visit_borrowed_bytes(bytes)
        } else {
            self.deserialize_any(visitor)
        }
    }

    serde::forward_to_deserialize_any! {
        bool i8 i16 i32 i64 i128 u8 u16 u32 u64 u128 f32 f64 char str string
        bytes byte_buf unit unit_struct seq tuple
        tuple_struct map struct enum identifier ignored_any
    }
}

impl<'de> de::Deserialize<'de> for VALUE {
    fn deserialize<D: serde::Deserializer<'de>>(d: D) -> std::result::Result<Self, D::Error> {
        use std::fmt;

        struct Visitor;

        impl<'de> de::Visitor<'de> for Visitor {
            type Value = VALUE;

            fn expecting(&self, f: &mut fmt::Formatter) -> fmt::Result {
                write!(f, "a pointer to a Ruby object")
            }

            fn visit_bytes<E>(self, bytes: &[u8]) -> std::result::Result<Self::Value, E>
            where
                E: de::Error,
            {
                if bytes.len() == size_of::<VALUE>() {
                    // Safety: The bytes must be a valid VALUE. We are assuming
                    // this is only ever called from the Deserializer in this file,
                    // or if another deserializer is used that it will not call
                    // `visit_borrowed_bytes` when we tell it to deserialize a
                    // newtype struct. We've checked the size as a last ditch
                    // protection
                    //
                    // This lint is allowed because the pointer was originally
                    // constructed from a `VALUE` and is known to be aligned.
                    #[allow(clippy::cast_ptr_alignment)]
                    unsafe {
                        Ok(*(bytes.as_ptr() as *const VALUE))
                    }
                } else {
                    Err(de::Error::custom("not a VALUE"))
                }
            }
        }

        d.deserialize_newtype_struct("VALUE", Visitor)
    }
}

impl VALUE {
    pub(crate) fn into_deserializer(self) -> impl serde::Deserializer<'static> + Copy {
        Deserializer(self)
    }
}

struct SeqAccess {
    arr: VALUE,
    idx: usize,
    len: usize,
}

impl SeqAccess {
    fn new(arr: VALUE) -> Self {
        let len = unsafe { ruby::rubyfmt_rb_ary_len(arr) } as usize;
        Self { arr, len, idx: 0 }
    }
}

impl<'de> de::SeqAccess<'de> for SeqAccess {
    type Error = Error;

    fn next_element_seed<T: de::DeserializeSeed<'de>>(
        &mut self,
        seed: T,
    ) -> Result<Option<T::Value>> {
        if self.idx < self.len {
            let elem = unsafe { ruby::rb_ary_entry(self.arr, self.idx as _) };
            self.idx += 1;
            seed.deserialize(Deserializer(elem)).map(Some)
        } else {
            Ok(None)
        }
    }

    fn size_hint(&self) -> Option<usize> {
        Some(self.len - self.idx)
    }
}

fn sym_to_str(v: VALUE) -> Result<&'static str> {
    use std::ffi::CStr;

    unsafe {
        let id = ruby::rb_sym2id(v);
        let c_str = CStr::from_ptr(ruby::rb_id2name(id));
        c_str.to_str().map_err(|_| invalid_utf8(c_str.to_bytes()))
    }
}

fn rstring_to_str(v: VALUE) -> Result<&'static str> {
    unsafe {
        let bytes = std::slice::from_raw_parts(
            ruby::rubyfmt_rstring_ptr(v) as _,
            ruby::rubyfmt_rstring_len(v) as _,
        );
        std::str::from_utf8(bytes).map_err(|_| invalid_utf8(bytes))
    }
}

fn invalid_utf8(bytes: &[u8]) -> Error {
    Error::invalid_value(de::Unexpected::Bytes(bytes), &"a valid UTF-8 string")
}
