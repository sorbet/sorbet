# typed: strict
# enable-experimental-rbs-comments: true

module Errors
  #: A = Integer
  #  ^ error: Failed to parse RBS signature (expected a token `pARROW`)

  #: e1 = ->
  #       ^^ error: Failed to parse RBS type (unexpected token for simple type)

  #: e2 = # error: Failed to parse RBS type (unexpected token for simple type)

  #: e3 = 123
  #       ^^^ error: RBS literal types are not supported

  #: -> void
  def foo
    #: a = Integer
  # ^^^^^^^^^^^^^^ error: Unexpected RBS assertion comment found in `method`
  end

  #: -> void
  def bar
    #: a = Integer
  # ^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
    foo
  end

  #: -> foo
  #     ^^^ error: Unable to resolve constant `<RBS alias>::foo`
  def baz; end

  #: [Elem]
  module Generic
    #: elem = Elem # error: Defining a `type_alias` to a generic `type_member` is not allowed

    #: -> elem
    def foo
      # behaves as untyped
    end
  end
end

module TypeAliasSimple
  #: a = Integer

  #: (a a) -> Integer
  def bar(a)
    T.reveal_type(a) # error: Revealed type: `Integer`
    a
  end
end

module TypeAliasUnion
  #: int_or_string = Integer | String

  #: (int_or_string) -> void
  def self.foo(x); end

  foo(1) # no error
  foo("foo") # no error
  foo(nil) # error: Expected `T.any(Integer, String)` but found `NilClass` for argument `x`
end

module TypeAliasOfTypeAlias
  #: a = Integer

  #: b = a

  #: (a a, b b) -> void
  def bar(a, b)
    T.reveal_type(a) # error: Revealed type: `Integer`
    T.reveal_type(b) # error: Revealed type: `Integer`
  end
end

module TypeAliasTuple
  #: tuple = [
  #|   Integer,
  #|   String,
  #|   Float,
  #| ]

  #: -> tuple
  def foo
    [1, "foo", 3.14]
  end

  #: -> tuple
  def bar
    []
  # ^^ error: Expected `[Integer, String, Float]` but found `[]` for method result type
  end
end

module TypeAliasSelf
  #: a = self

  #: -> a
  def foo
    self
  end
end
