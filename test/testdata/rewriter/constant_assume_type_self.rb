# typed: strict

class Foo
  BAR = new
  EXPLICIT = self.new
  FROZEN = new.freeze
  EXPLICIT_FROZEN = self.new.freeze
end

T.reveal_type(Foo::BAR) # error: `Foo`
T.reveal_type(Foo::EXPLICIT) # error: `Foo`
T.reveal_type(Foo::FROZEN) # error: `Foo`
T.reveal_type(Foo::EXPLICIT_FROZEN) # error: `Foo`

class WithArguments
  extend T::Sig

  sig {params(value: Integer, name: String).void}
  def initialize(value, name:)
  end

  INSTANCE = new(1, name: "one")
end

T.reveal_type(WithArguments::INSTANCE) # error: `WithArguments`

module Nested
  class Foo
    # The enclosing class's name is shadowed inside its own body.
    class Foo
    end

    INSTANCE = new
  end
end

T.reveal_type(Nested::Foo::INSTANCE) # error: `Nested::Foo`

class Nested::Qualified
  INSTANCE = new
end

T.reveal_type(Nested::Qualified::INSTANCE) # error: `Nested::Qualified`

class Child < Foo
  INSTANCE = new
end

T.reveal_type(Child::INSTANCE) # error: `Child`
