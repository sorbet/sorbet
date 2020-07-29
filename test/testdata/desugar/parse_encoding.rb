
# typed: strict

x = __ENCODING__
T.reveal_type(x) # error: Revealed type: `Encoding`
T.reveal_type(__ENCODING__) # error: Revealed type: `Encoding`
T.reveal_type(__ENCODING__.name) # error: Revealed type: `String`
T.reveal_type(__ENCODING__.names) # error: Revealed type: `T::Array[String]`
T.reveal_type(__ENCODING__.to_s) # error: Revealed type: `String`
T.reveal_type(__ENCODING__.dummy?) # error: Revealed type: `T::Boolean`
