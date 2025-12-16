# typed: strict
# enable-experimental-rbs-comments: true

module Errors
  #: A = Integer
  #  ^ error: Failed to parse RBS signature (expected a token `pARROW`)

  #: type e1 = ->
  #            ^^ error: Failed to parse RBS type (unexpected token for simple type)

  #: type e2 = # error: Failed to parse RBS type (unexpected token for simple type)

  #: type e3 = 123
  #            ^^^ error: RBS literal types are not supported

  #: -> void
  def foo
    #: type a = Integer
  # ^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
  end

  #: -> void
  def bar
    #: type a = Integer
  # ^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS type alias comment
    foo
  end

  #: -> foo
  #     ^^^ error: Unable to resolve constant `type foo`
  def baz; end

  #: [Elem]
  module Generic
    #: type elem = Elem # error: Defining a `type_alias` to a generic `type_member` is not allowed

    #: -> elem
    def foo
      # behaves as untyped
    end
  end

  #: type e_multiline1
# ^^^^^^^^^^^^^^^^^^^^ error: Unexpected RBS assertion comment found in `module`
  #|   = Integer
# ^^^^^^^^^^^^^^ error: Unexpected RBS assertion comment found in `module`
  #|   | String
# ^^^^^^^^^^^^^ error: Unexpected RBS assertion comment found in `module`
end

module TypeAliasSimple
  #: type a = Integer

  #: (a a) -> Integer
  def bar(a)
    T.reveal_type(a) # error: Revealed type: `Integer`
    a
  end
end

module TypeAliasUnion
  #: type int_or_string = Integer | String

  #: (int_or_string) -> void
  def self.foo(x); end

  foo(1) # no error
  foo("foo") # no error
  foo(nil) # error: Expected `T.any(Integer, String)` but found `NilClass` for argument `x`
end

module TypeAliasOfTypeAlias
  #: type a = Integer

  #: type b = a

  #: (a a, b b) -> void
  def bar(a, b)
    T.reveal_type(a) # error: Revealed type: `Integer`
    T.reveal_type(b) # error: Revealed type: `Integer`
  end
end

module TypeAliasTuple
  #: type tuple = [
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
  #: type a = self

  #: -> a
  def foo
    self
  end
end

module TypeAliasMultiline
  #: type a = Integer
  #|   | String

  #: (a) -> void
  def method1(x)
    T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
  end

  #: type b = Integer
  #|        | String

  #: (b) -> void
  def method2(x)
    T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
  end

  #: type c =
  #|          Integer
  #|        | String

  #: (c) -> void
  def method3(x)
    T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
  end
end

module TypeAliasWithNamespace
  module Foo
    #: type a = Integer | String

    #: (a) -> void
    def foo(x)
      T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
    end
  end

  #: (Foo::a) -> void
  def bar(x)
    T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
  end
end

module TrailingTypeAlias
  CONSTANT = 1

  #: type a = Integer
end

module EmptyTypeAlias
  #: type a = Integer
end
