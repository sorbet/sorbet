# typed: strict

extend T::Sig

sig {returns(T.untyped)}
attr_writer :foo # error: Malformed `sig`. Type not specified for argument `foo`
