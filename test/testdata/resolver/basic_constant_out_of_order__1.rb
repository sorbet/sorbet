# check-out-of-order-constant-references: true
# typed: false

module Foo
  A = X 
  #   ^ error: `Foo::X` referenced before it is defined

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
end
