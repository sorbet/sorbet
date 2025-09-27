# typed: strict
# enable-experimental-rbs-comments: true

extend T::Sig

class P1; end
class P2; end
class P3; end
class P4; end
class P5; end
class P6; end

#: (P1, P2
#| -> void
#  ^^ error: Failed to parse RBS signature (unexpected token for function parameter name)
def parse_error1; T.unsafe(nil); end # error: The method `parse_error1` does not have a `sig`

#: (P1,
#| P2
#|   -> void
#    ^^ error: Failed to parse RBS signature (unexpected token for function parameter name)
def parse_error2; T.unsafe(nil); end # error: The method `parse_error2` does not have a `sig`

#: (
#| P1,
#|  P2
#| -> void
#  ^^ error: Failed to parse RBS signature (unexpected token for function parameter name)
def parse_error3; T.unsafe(nil); end # error: The method `parse_error3` does not have a `sig`

#: (
#|   String x ) -> String
#           ^ error: Unknown parameter name `x`
def named_args1(y)
  #             ^ error: Malformed `sig`. Type not specified for parameter `y`
  T.reveal_type(y) # error: Revealed type: `T.untyped`
end

#: (
#|   String
#| x ) -> String
#  ^ error: Unknown parameter name `x`
def named_args2(y)
  #             ^ error: Malformed `sig`. Type not specified for parameter `y`
  T.reveal_type(y) # error: Revealed type: `T.untyped`
end

#: ->
#| String
def method2; T.unsafe(nil); end
T.reveal_type(method2) # error: Revealed type: `String`

#:      ->
#| String
def method4; T.unsafe(nil); end
T.reveal_type(method4) # error: Revealed type: `String`

# some comment
#: ->
#| String
# some comment
def method5; T.unsafe(nil); end
T.reveal_type(method5) # error: Revealed type: `String`

  #: -> String
# ^^^^^^^^^^^^ error: Unused type annotation. No method def before next annotation
  #: ->
  #| void
  def method6; T.unsafe(nil); end
  T.reveal_type(method6) # error: Revealed type: `Sorbet::Private::Static::Void`

#: (P1) ->
#| P10
#  ^^^ error: Unable to resolve constant `P10`
def method_with_missing_type_1(p1)
  T.reveal_type(p1) # error: Revealed type: `P1`
end

#: [X] (
#|   X & Object
#| ) -> Class[Y]
#             ^ error: Unable to resolve constant `Y`
def method_with_missing_type_2(x)
  T.reveal_type(x) # error: Revealed type: `T.all(Object, T.type_parameter(:X) (of Object#method_with_missing_type_2))`
  x.class
end

#: (P1) ->
#| void
def method7(p1)
  T.reveal_type(p1) # error: Revealed type: `P1`
end
method7(P1.new)
method7(42) # error: Expected `P1` but found `Integer(42)` for argument `p1`

#: (
#|   P1,
#|   P2
#| ) -> void
def method8(p1, p2)
  T.reveal_type(p1) # error: Revealed type: `P1`
  T.reveal_type(p2) # error: Revealed type: `P2`
end
method8(P1.new, 42) # error: Expected `P2` but found `Integer(42)` for argument `p2`

#: (
#|   ?Integer
#| ) -> void
def method9(p1 = 42)
  T.reveal_type(p1) # error: Revealed type: `Integer`
end
method9
method9(42)

#: (
#|   P1,
#|   ?P2?
#| ) -> void
def method10(p1, p2 = nil)
  T.reveal_type(p1) # error: Revealed type: `P1`
  T.reveal_type(p2) # error: Revealed type: `T.nilable(P2)`
end
method10(P1.new, nil)
method10(P1.new, P2.new)
method10(P1.new)

# Named args

#: (
#|   String x
#| ) -> String
def method11(x)
  T.reveal_type(x) # error: Revealed type: `String`
end
method11("foo")


#: (
#|   String   ?x
#| ) -> void
def method13(x)
  T.reveal_type(x) # error: Revealed type: `T.nilable(String)`
end

#: (
#|   ?String x
#| ) -> void
def method14(x = "")
  T.reveal_type(x) # error: Revealed type: `String`
end

#: (
#|   String ?x
#| ) -> void
def method16(x)
  T.reveal_type(x) # error: Revealed type: `T.nilable(String)`
end

#: ?{ -> void } ->
#| void
def method18(&block)
  T.reveal_type(block) # error: Revealed type: `T.nilable(T.proc.void)`
end

#: [X] (
#|   X & Object
#| ) -> Class[X]
def method19(x)
  T.reveal_type(x) # error: Revealed type: `T.all(Object, T.type_parameter(:X) (of Object#method19))`
  x.class
end
T.reveal_type(method19(42)) # error: Revealed type: `T::Class[Integer]`

#: ?{
#|   (?) -> untyped
#| } -> void
def method20(&block)
  T.reveal_type(block) # error: Revealed type: `T.untyped`
end

class FooProc
  #: (?p: ^() -> Integer)
  #| ?{
  #|   (Integer) [self: FooProc] -> String
  #| } -> void
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

# comment
#: (
# comment
#| P1,
# comment
#| P2
# comment
#| )
# comment
#| ->
# comment
#| P10
#  ^^^ error: Unable to resolve constant `P10`
def method21(x, y); end

#| -> void # error: Multiline signature ("#|") must be preceded by a signature start ("#:")
def method22; end # error: The method `method22` does not have a `sig`

#: (
#|   P1,
#|   P2
#: ) -> void # error: Signature start ("#:") cannot appear after a multiline signature ("#|")
def method23(p1, p2); end # error: The method `method23` does not have a `sig`
