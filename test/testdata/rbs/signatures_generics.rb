# typed: strict
# enable-experimental-rbs-comments: true

module Errors
  #:
  # ^ error: Failed to parse RBS type parameters (expected a token `pLBRACKET`)
  class ParseError1; end

  #: -> void
  #  ^^ error: Failed to parse RBS type parameters (expected a token `pLBRACKET`)
  class ParseError2; end

  #: [
  # ^^ error: Failed to parse RBS type parameters (expected a token `tUIDENT`)
  class ParseError3; end

  #: [T
  # ^^^ error: Failed to parse RBS type parameters (expected ',' or ']' after type parameter, got pEOF)
  class ParseError4; end

  #: [unchecked T]
  #   ^^^^^^^^^^^ error: `unchecked` type parameters are not supported by Sorbet
  class UnsupportedError1; end

  class UselessSignature1 #: [U]
  #                       ^^^^^^ error: Unexpected RBS assertion comment found after `class` declaration
  end
end

#: [U]
class G1; end

g1_1 = G1.new
T.reveal_type(g1_1) # error: Revealed type: `G1[T.untyped]`

g1_2 = G1.new #: G1[Integer]
T.reveal_type(g1_2) # error: Revealed type: `G1[Integer]`

g1_3 = G1.new #: G1[String]
T.reveal_type(g1_3) # error: Revealed type: `G1[String]`

#: (G1) -> void
#   ^^ error: Malformed type declaration. Generic class without type arguments `G1`
def take_g1_1(g1)
  T.reveal_type(g1) # error: Revealed type: `G1[T.untyped]`
end

#: (G1[Integer]) -> void
def take_g1_2(g1)
  T.reveal_type(g1) # error: Revealed type: `G1[Integer]`
end

take_g1_2(g1_1)
take_g1_2(g1_2)
take_g1_2(g1_3) # error: Expected `G1[Integer]` but found `G1[String]` for argument `g1`

#: [out U, V]
class G2
  #: U
  attr_reader :u

  #: V
  attr_accessor :v

  #: (U, V) -> void
  def initialize(u, v)
    @u = u
    @v = v
  end
end

g2_1 = G2.new(1, 2) #: G2[Integer, String]
#                ^ error: Expected `String` but found `Integer(2)` for argument `v`
T.reveal_type(g2_1) # error: Revealed type: `G2[Integer, String]`

g2_1.v = 2 # error: Assigning a value to `v` that does not match expected type `String`

g2_2 = G2 #: as Class[G2[Integer, String]]
   .new("a", 2) #: G2[Numeric, String]
#       ^^^ error: Expected `Integer` but found `String("a")` for argument `u`
#            ^ error: Expected `String` but found `Integer(2)` for argument `v`
T.reveal_type(g2_2) # error: Revealed type: `G2[Numeric, String]`

g2_3 = G2 #: as Class[G2[Integer, String]]
   .new(1, 2) #: G2[Integer, String]?
#          ^ error: Expected `String` but found `Integer(2)` for argument `v`
T.reveal_type(g2_3) # error: Revealed type: `T.nilable(G2[Integer, String])`

#: [in U, out V]
class G3
  #: U
  attr_reader :u # error: `type_member` `U` was defined as `:in` but is used in an `:out` context

  #: V
  attr_writer :v # error: `type_member` `V` was defined as `:out` but is used in an `:in` context

  #: (U, V) -> void
  def initialize(u, v)
    @u = u
    @v = v
  end
end

#: [U < String]
class G4; end

g4 = G4.new #: G4[Integer]
#                 ^^^^^^^ error-with-dupes: `Integer` is not a subtype of upper bound of type member `::G4::U`
T.reveal_type(g4) # error: Revealed type: `G4[T.untyped]`

#: [U > BasicObject]
class G5; end

g5 = G5.new #: G5[String]
#                 ^^^^^^ error-with-dupes: `String` is not a supertype of lower bound of type member `::G5::U`
T.reveal_type(g5) # error: Revealed type: `G5[T.untyped]`

#: [U]
class G6; end

g6 = G6[Integer].new # error: Method `[]` does not exist on `T.class_of(G6)`
T.reveal_type(g6) # error: Revealed type: `T.untyped`

arr1 = Array.new #: T::Array[Integer]
T.reveal_type(arr1) # error: Revealed type: `T::Array[Integer]`

arr2 = T::Array[Integer].new(1)
T.reveal_type(arr2) # error: Revealed type: `T::Array[Integer]`

#: [U < Numeric]
class G7; end

#: [out E]
class G8; end
g8_1 = G8.new #: G8[Numeric]
T.reveal_type(g8_1) # error: Revealed type: `G8[Numeric]`

g8_2 = g8_1 #: G8[Object]
T.reveal_type(g8_2) # error: Revealed type: `G8[Object]`

g8_3 = g8_1 #: G8[Integer]
#              ^^^^^^^^^^^ error: Argument does not have asserted type `G8[Integer]`
T.reveal_type(g8_3) # error: Revealed type: `G8[Integer]`

#: [in E]
class G9; end
g9_1 = G9.new #: G9[Numeric]
T.reveal_type(g9_1) # error: Revealed type: `G9[Numeric]`

g9_2 = g9_1 #: G9[Object]
#              ^^^^^^^^^^ error: Argument does not have asserted type `G9[Object]`
T.reveal_type(g9_2) # error: Revealed type: `G9[Object]`

g9_3 = g9_1 #: G9[Integer]
T.reveal_type(g9_3) # error: Revealed type: `G9[Integer]`

#: [U]
class G10
  extend T::Generic

  V = type_member
end

g10 = G10[Integer, String].new #: G10[Integer, String]
T.reveal_type(g10) # error: Revealed type: `G10[Integer, String]`

#: [U < Numeric = Integer]
#   ^^^^^^^^^^^^^^^^^^^^^ error: Type member is defined with bounds and `fixed`
class G11; end

#: [in U < Numeric]
class G12; end

#: (G12[Object]) -> void
#       ^^^^^^ error-with-dupes: `Object` is not a subtype of upper bound of type member `::G12::U`
def take_g12(g12); end

#: [
#|   A,
#|   B
#| ]
class G13
  #: (A | B) -> void
  def foo(x)
    T.reveal_type(x) # error: Revealed type: `T.any(G13::A, G13::B)`
  end
end

x = G13.new #: G13[Integer, String]
x.foo(1)
x.foo("a")
x.foo(nil) # error: Expected `T.any(Integer, String)` but found `NilClass` for argument `x`

# Test deep copy with union types
#: [U]
class G14; end

g14 = G14.new #: G14[Integer | String]
T.reveal_type(g14) # error: Revealed type: `G14[T.any(Integer, String)]`

# Test deep copy with intersection types
g15 = G14.new #: G14[Numeric & Comparable]
T.reveal_type(g15) # error: Revealed type: `G14[T.all(Numeric, Comparable)]`

# Test deep copy with nested constant path
module MyModule
  class MyClass; end
end

g16 = G14.new #: G14[MyModule::MyClass]
T.reveal_type(g16) # error: Revealed type: `G14[MyModule::MyClass]`

# Test deep copy with type parameter references
#: [T, U]
class G17; end

#: [X] (X) -> G17[X, X]
def make_g17(x)
  G17.new #: G17[X, X]
end

#: [X = void]
class G18; end
