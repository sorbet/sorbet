# typed: strict

extend T::Sig

class OpaqueString < T::Types::Opaque
  ConcreteType = type_template(fixed: String)
end

sig {params(x: OpaqueString).void}
def takes_opaque_type(x); end

sig {params(x: String).void}
def takes_concrete_type(x); end

takes_opaque_type(OpaqueString.let(""))
takes_opaque_type("") # error: Expected `OpaqueString` but found `String("")` for argument `x`
takes_concrete_type(OpaqueString.let("")) # error: Expected `String` but found `OpaqueString` for argument `x`
takes_concrete_type("")
