# typed: strict
# enable-experimental-rbs-signatures: true

extend T::Sig

class P1; end
class P2; end
class P3; end
class P4; end
class P5; end
class P6; end

# Parse errors

#: (P1, P2 -> void
#          ^^ error: Failed to parse RBS signature (unexpected token for function parameter name)
def parse_error1; T.unsafe(nil); end # error: The method `parse_error1` does not have a `sig`

class ParseError2
  #: void
  #  ^^^^ error: Failed to parse RBS signature (expected a token `pARROW`)
  def parse_error2(p1, p2); end # error: The method `parse_error2` does not have a `sig`

  class ParseError3
    # Some comment
    #: v
    #  ^ error: Failed to parse RBS signature (expected a token `pARROW`)
    # Some comment
    def parse_error3(p1, p2); end # error: The method `parse_error3` does not have a `sig`
  end
end

  #:
 #  ^ error: Failed to parse RBS signature (expected a token `pARROW`)
  def parse_error4(p1, p2); end # error: The method `parse_error4` does not have a `sig`

# Sig mismatch

#: (P1) -> void
def sig_mismatch1(p1, p2); end
#                     ^^ error: Malformed `sig`. Type not specified for argument `p2`

#: (foo: P1) -> void # error: Unknown argument name `foo`
def sig_mismatch2(p1); end
#                 ^^ error: Malformed `sig`. Type not specified for argument `p1`

#: (P1, P2, P3) -> void # error: RBS signature has more parameters than in the method definition
def sig_mismatch3; end # error: The method `sig_mismatch3` does not have a `sig`

# Sigs

# We do not create any sig if there is no RBS comment
def method1; T.unsafe(nil); end # error: The method `method1` does not have a `sig`

#: -> String
def method2; T.unsafe(nil); end
T.reveal_type(method2) # error: Revealed type: `String`

#:-> String
def method3; T.unsafe(nil); end
T.reveal_type(method3) # error: Revealed type: `String`

#:      -> String
def method4; T.unsafe(nil); end
T.reveal_type(method4) # error: Revealed type: `String`

# some comment
#: -> String
# some comment
def method5; T.unsafe(nil); end
T.reveal_type(method5) # error: Revealed type: `String`

  #: -> String
# ^^^^^^^^^^^^ error: Unused type annotation. No method def before next annotation
  #: -> void
  def method6; T.unsafe(nil); end
  T.reveal_type(method6) # error: Revealed type: `Sorbet::Private::Static::Void`

#: (P1) -> void
def method7(p1)
  T.reveal_type(p1) # error: Revealed type: `P1`
end
method7(P1.new)
method7(42) # error: Expected `P1` but found `Integer(42)` for argument `P1`

#: (P1, P2) -> void
def method8(p1, p2)
  T.reveal_type(p1) # error: Revealed type: `P1`
  T.reveal_type(p2) # error: Revealed type: `P2`
end
method8(P1.new, 42) # error: Expected `P2` but found `Integer(42)` for argument `P2`


#: (?Integer) -> void
def method9(p1 = 42)
  T.reveal_type(p1) # error: Revealed type: `Integer`
end
method9
method9(42)

#: (P1, ?P2?) -> void
def method10(p1, p2 = nil)
  T.reveal_type(p1) # error: Revealed type: `P1`
  T.reveal_type(p2) # error: Revealed type: `T.nilable(P2)`
end
method10(P1.new, nil)
method10(P1.new, P2.new)
method10(P1.new)

# Named args

#: (String x) -> String
def method11(x)
  T.reveal_type(x) # error: Revealed type: `String`
end
method11("foo")
method11(42) # error: Expected `String` but found `Integer(42)` for argument `String x`

#: (String x) -> String
#   ^^^^^^^^ error: Unknown argument name `x`
def named_args2(y)
  #             ^ error: Malformed `sig`. Type not specified for argument `y`
  T.reveal_type(y) # error: Revealed type: `T.untyped`
end

#: (String foo) -> String
#   ^^^^^^^^^^ error: Unknown argument name `foo`
def method12(y)
  #          ^ error: Malformed `sig`. Type not specified for argument `y`
  T.reveal_type(y) # error: Revealed type: `T.untyped`
end

#: (String ?x) -> void
def method13(x)
  T.reveal_type(x) # error: Revealed type: `T.nilable(String)`
end

#: (?String x) -> void
def method14(x)
  T.reveal_type(x) # error: Revealed type: `String`
end

#: (String x) -> void
def method15(x = nil) # error: Argument does not have asserted type `String`
  T.reveal_type(x) # error: Revealed type: `T.nilable(String)`
end

#: (String ?x) -> void
def method16(x = nil)
  T.reveal_type(x) # error: Revealed type: `T.nilable(String)`
end

#: (P1, ?P2?, *P3, p4: P4, ?p5: P5?, **P6) { -> void } -> void
def method17(p1, p2 = nil, *p3, p4:, p5: nil, **p6, &block)
  T.reveal_type(p1) # error: Revealed type: `P1`
  T.reveal_type(p2) # error: Revealed type: `T.nilable(P2)`
  T.reveal_type(p3) # error: Revealed type: `T::Array[P3]`
  T.reveal_type(p4) # error: Revealed type: `P4`
  T.reveal_type(p5) # error: Revealed type: `T.nilable(P5)`
  T.reveal_type(p6) # error: Revealed type: `T::Hash[Symbol, P6]`
  T.reveal_type(block) # error: Revealed type: `T.proc.void`
end
method17(P1.new, P2.new, P3.new, p4: 42, p5: P5.new, p6: P6.new) {} # error: Expected `P4` but found `Integer(42)` for argument `p4`
method17(P1.new, P2.new, P3.new, p4: P4.new, p5: 42, p6: P6.new) {} # error: Expected `T.nilable(P5)` but found `Integer(42)` for argument `p5`
method17(P1.new, P2.new, P3.new, p4: P4.new, p5: P5.new, p6: 42) {} # error: Expected `P6` but found `Integer(42)` for argument `P6`

#: ?{ -> void } -> void
def method18(&block)
  T.reveal_type(block) # error: Revealed type: `T.nilable(T.proc.void)`
end

#: [X] (X & Object) -> Class[X]
def method19(x)
  T.reveal_type(x) # error: Revealed type: `T.all(Object, T.type_parameter(:X) (of Object#method19))`
  x.class
end
T.reveal_type(method19(42)) # error: Revealed type: `T::Class[Integer]`

#: ?{ (?) -> untyped } -> void
def method20(&block)
  T.reveal_type(block) # error: Revealed type: `T.untyped`
end

class FooProc
  #: (p: ^() -> Integer ) ?{ (Integer) [self: FooProc] -> String } -> void
  def initialize(p: -> { 42 }, &block)
    T.reveal_type(p) # error: Revealed type: `T.proc.returns(Integer)`
    T.reveal_type(block) # error: Revealed type: `T.nilable(T.proc.params(arg0: Integer).returns(String))`
    T.reveal_type(p.call) # error: Revealed type: `Integer`
    T.reveal_type(block&.call(42)) # error: Revealed type: `T.nilable(String)`
  end
end

FooProc.new do |foo|
  T.reveal_type(self) # error: Revealed type: `FooProc`
  T.reveal_type(foo) # error: Revealed type: `Integer`
  "foo"
end

module Annotations
  # @abstract
  #: -> Integer
  def method1; end # error: Before declaring an abstract method, you must mark your class/module as abstract using `abstract!` or `interface!`

  # @override
  #: -> Integer
  def method2; T.unsafe(nil); end # error: Method `Annotations#method2` is marked `override` but does not override anything

  class Parent
    #: (Integer) -> void
    def method(x)
      T.reveal_type(x) # error: Revealed type: `Integer`
    end
  end

  class OverrideIncompatible < Parent
    # @override
    #: (String) -> void
    def method(x) # error: Parameter `String` of type `String` not compatible with type of overridden method `Annotations::Parent#method`
      T.reveal_type(x) # error: Revealed type: `String`
    end
  end

  class OverrideAllowIncompatible < Parent
    # @override(allow_incompatible: true)
    #: (String) -> void
    def method(x)
      T.reveal_type(x) # error: Revealed type: `String`
    end
  end

  class Final
    extend T::Helpers

    final!

    # @final
    #: -> void
    def foo; end
  end
end

class Visibility
  #: (Integer) -> void
  private def method1(x)
    T.reveal_type(x) # error: Revealed type: `Integer`
  end

  #: (Integer) -> void
  protected def method2(x)
    T.reveal_type(x) # error: Revealed type: `Integer`
  end

  #: (Integer) -> void
  public def method3(x)
    T.reveal_type(x) # error: Revealed type: `Integer`
  end
end
