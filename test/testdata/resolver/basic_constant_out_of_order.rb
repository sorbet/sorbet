# stripe-mode: true
# typed: true

module Foo
  A = X 
    # ^ error: `Foo::X` referenced before it is declared

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

  class X; end
end
