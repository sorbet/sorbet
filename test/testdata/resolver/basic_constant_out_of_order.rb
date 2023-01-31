# enable-out-of-order-reference-checks: true
# typed: true

module Foo
  A = X 
    # ^ error: `Foo::X` referenced before it is defined

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
    Foo::Y
  # ^^^^^^ error: `Foo::Y` referenced before it is defined
  end

  class X; end

  class Bar
    Foo.bar do
      Foo::Y # this is ok
    end
  end

  Y = 1

  class Bar
    Foo::Y # this is ok
  end

  Y = 2
end
