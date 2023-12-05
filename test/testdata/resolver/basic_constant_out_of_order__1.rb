# check-out-of-order-constant-references: true
# typed: false

module Foo
  # Sorbet doesn't report an error because Foo::X also has a definition in the RBI file.
  A = X

  def self.foo(arg:)
    X # this is ok
  end

  def self.bar(&blk)
    X # this is ok
  end

  foo arg: ->{ X } # this is ok

  bar do
    X # this is ok
  end

  class X; end

  B = X # this is ok

  class Bar
    p(Foo::Y)
    # ^^^^^^ error: `Foo::Y` referenced before it is defined
  end

  class X; end

  class Bar
    Foo.bar do
      p(Foo::Y) # this is ok
    end
  end

  Y = 1

  Y = 2

  class Bar
    p(Foo::Y) # this is ok
  end

  Y = 3

  p(Foo::Z) # this is ok since Foo::Z is also defined in another file
  class Z
  end
end
