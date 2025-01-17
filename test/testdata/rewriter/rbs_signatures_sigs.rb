# typed: strict
# enable-experimental-rbs-signatures: true

extend T::Sig

module P1; end
module P2; end
module P3; end
module P4; end
module P5; end
module P6; end

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

# Sigs

# We do not create any RBI if there is no RBS sigs
def method1; T.unsafe(nil); end # error: The method `method1` does not have a `sig`

#: -> String
def method2; T.unsafe(nil); end

#: -> String
def method3; T.unsafe(nil); end

#:      -> String
def method4; T.unsafe(nil); end

# some comment
#: -> String
# some comment
def method5; T.unsafe(nil); end

  #: -> String
# ^^^^^^^^^^^^ error: Unused type annotation. No method def before next annotation
  #: -> void
  def method6; T.unsafe(nil); end

#: (P1) -> void
def method7(p1)
  T.reveal_type(p1) # error: Revealed type: `P1`
end

#: (P1, P2) -> void
def method8(p1, p2)
  T.reveal_type(p1) # error: Revealed type: `P1`
  T.reveal_type(p2) # error: Revealed type: `P2`
end

#: (?Integer) -> void
def method9(p1 = 42)
  T.reveal_type(p1) # error: Revealed type: `Integer`
end

#: (?P1?, P2) -> void
def method10(p1 = nil, p2)
  T.reveal_type(p1) # error: Revealed type: `T.nilable(P1)`
  T.reveal_type(p2) # error: Revealed type: `P2`
end

# Named args

#: (String x) -> String
def method11(x)
  T.reveal_type(x) # error: Revealed type: `String`
end

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

#: ?{ -> void } -> void
def method18(&block)
  T.reveal_type(block) # error: Revealed type: `T.nilable(T.proc.void)`
end

#: [X] (X & Object) -> Class[X]
def method19(x)
  T.reveal_type(x) # error: Revealed type: `T.all(Object, T.type_parameter(:X) (of Object#method19))`
  x.class
end

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
