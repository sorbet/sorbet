# typed: true
# enable-experimental-rbs-comments: true

#: self as Foo
T.reveal_type(self) # error: Revealed type: `Foo`

class Foo
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
end

module Bar
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`

  class << self
    #: self as Foo
    T.reveal_type(self) # error: Revealed type: `Foo`
  end
end

class Baz; end

def foo
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
end

def bar
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end

def self.baz
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
end

[].each do
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
end

case ARGV.first
when "foo"
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
else
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end

case ARGV.first
in "foo"
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
else
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end

for _i in 0..10
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
end

if ARGV.first == "foo"
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
elsif ARGV.first == "bar"
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
else
  #: self as untyped
  T.reveal_type(self) # error: Revealed type: `T.untyped`
end

if ARGV.first == "foo"
  [].each do
    #: self as Foo
    T.reveal_type(self) # error: Revealed type: `Foo`
  end
else
  [].each do
    #: self as Foo
    T.reveal_type(self) # error: Revealed type: `Foo`
  end
end

begin
  #: self as Foo
  T.reveal_type(self) # error: Revealed type: `Foo`
rescue
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
else
  #: self as Baz
  T.reveal_type(self) # error: Revealed type: `Baz`
ensure
  #: self as untyped
  T.reveal_type(self) # error: Revealed type: `T.untyped`
end

until ARGV.first == "foo"
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end

begin
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end until ARGV.first == "foo"

while ARGV.first == "foo"
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end

begin
  #: self as Bar
  T.reveal_type(self) # error: Revealed type: `Bar`
end while ARGV.first == "foo"

module TypeAlias
  #: type foo = String
  #: self as foo
  T.reveal_type(self) # error: Revealed type: `String`
end

module Errors
  #: self as 12
  #          ^^ error: RBS literal types are not supported

  #: self as (
  #          ^ error: Failed to parse RBS type (unexpected token for simple type)

  T.reveal_type(self) # error: Revealed type: `T.untyped`

  self #: self as String
  #               ^^^^^^ error: `self` binding can't be used as a trailing comment
end

#: self as Bar
T.reveal_type(self) # error: Revealed type: `Bar`
