# typed: strict
# enable-experimental-rbs-signatures: true
# enable-experimental-rbs-assertions: true

# errors

e1 = ARGV #: Integer
          #  ^^^^^^^ error: Argument does not have asserted type `Integer`

e2 = ARGV #:Integer
          # ^^^^^^^ error: Argument does not have asserted type `Integer`

e3 = ARGV #: Int
          #  ^^^ error: Unable to resolve constant `Int`

e4 = ARGV #: -
          #  ^ error: Failed to parse RBS type (unexpected token for simple type)

e5 = ARGV #:
          # ^ error: Failed to parse RBS type (unexpected token for simple type)

e6 = ARGV #: Integer # error: Argument does not have asserted type `Integer`

# local variables

a = ARGV.first #: String
T.reveal_type(a) # error: Revealed type: `String`

b = nil #: Integer?
T.reveal_type(b) # error: Revealed type: `T.nilable(Integer)`

c =
    nil #: Integer?
T.reveal_type(c) # error: Revealed type: `T.nilable(Integer)`

d = ARGV
      .first
      .strip #: String
T.reveal_type(d) # error: Revealed type: `String`

e ||= "foo" #: String?
T.reveal_type(e) # error: Revealed type: `T.nilable(String)`

f = "" #: String?
f &&= "foo" #: Integer # error: Argument does not have asserted type `Integer`
T.reveal_type(f) # error: Revealed type: `T.nilable(Integer)`

g, *h = ARGV #: Array[String]
T.reveal_type(g) # error: Revealed type: `T.nilable(String)`
T.reveal_type(h) # error: Revealed type: `T::Array[String]`

i = "#: Integer" #: String
T.reveal_type(i) # error: Revealed type: `String`

j = 42 #: Integer
j += "" #: String # error: Expected `Integer` but found `String` for argument `arg0`
T.reveal_type(j) # error: Revealed type: `Integer`

# constants

A = ARGV.first #: String
T.reveal_type(A) # error: Revealed type: `String`

B = T.must("foo"[0]) #: String
T.reveal_type(B) # error: Revealed type: `String`

# instance variables

@a = ARGV.first #: String
T.reveal_type(@a) # error: Revealed type: `String`

@b = ARGV.first || [] #: Array[String]
T.reveal_type(@b) # error: Revealed type: `T::Array[String]`

@c = ARGV.first&.strip #: String?
T.reveal_type(@c) # error: Revealed type: `T.nilable(String)`

# class variables

@@a = 1 #: Integer
T.reveal_type(@@a) # error: Revealed type: `Integer`

# global variables

$a = ARGV.first #: String
T.reveal_type($a) # error: Revealed type: `String`

# accessors

class LetAccessor
  #: Integer
  attr_accessor :x

  #: (untyped) -> void
  def initialize(something)
    @x = something.num #: Integer
    T.reveal_type(@x) # error: Revealed type: `Integer`
    T.reveal_type(x) # error: Revealed type: `Integer`
  end
end

class DesugaredLetNilable
  #: -> Integer
  def foo
    @x ||= 42 #: Integer?
    T.reveal_type(@x) # error: Revealed type: `Integer`
  end

  #: -> void
  def bar
    @y ||= nil #: Integer?
    @y ||= 42
    T.reveal_type(@y) # error: Revealed type: `Integer`
  end

  #: -> void
  def baz
    T.reveal_type(@x) # error: Revealed type: `T.nilable(Integer)`
    T.reveal_type(@y) # error: Revealed type: `T.nilable(Integer)`
  end
end

# type parameters

class TypeParams
  #: [A, B, C] (A, B?, C) -> void
  def foo(a, b, c)
    x = a #: A
    T.reveal_type(x) # error: Revealed type: `T.type_parameter(:A) (of TypeParams#foo)`

    y = [] #: Array[B]
    T.reveal_type(y) # error: Revealed type: `T::Array[T.type_parameter(:B) (of TypeParams#foo)]`

    z = nil #: C?
    T.reveal_type(z) # error: Revealed type: `T.nilable(T.type_parameter(:C) (of TypeParams#foo))`
  end
end

# heredocs

# Make sure we don't parse parts of heredocs as RBS assertions
HEREDOC_ERROR = <<~MSG.strip # error: Constants must have type annotations with `T.let` when specifying `# typed: strict
  #: Integer
MSG

# Make sure we don't parse parts of strings as RBS assertions
HEREDOC_STRING = "<<~ #: Integer".strip # error: Constants must have type annotations with `T.let` when specifying `# typed: strict

HEREDOC1 = <<~MSG #: String?
  foo
MSG
T.reveal_type(HEREDOC1) # error: Revealed type: `T.nilable(String)`

HEREDOC2 = <<~MSG.strip.strip #: String?
  #{42}
MSG
T.reveal_type(HEREDOC2) # error: Revealed type: `T.nilable(String)`

HEREDOC3 = String(<<~MSG.strip) #: String?
  foo
MSG
T.reveal_type(HEREDOC3) # error: Revealed type: `T.nilable(String)`

HEREDOC4 = <<~MSG #: String?
  foo
  bar
  baz
MSG
T.reveal_type(HEREDOC4) # error: Revealed type: `T.nilable(String)`

# avoid desugar magics

class Foo
  #: (untyped) -> void
  def initialize(x)
    @x = x
  end
end

#: -> Hash[Symbol, untyped]
def foo_params
  {}
end

foo = Foo.new({
  **foo_params,
}) #: Foo?
