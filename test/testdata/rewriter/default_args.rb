# typed: true

class A
  extend T::Sig

  sig {params(a: String, b: Integer, c: Integer).void}
  def foo(a, b=1, c=2)
  end
end

class B
  extend T::Sig

  sig {returns(String)}
  attr_reader :foo

  sig {params(x: String).void}
  def initialize(x="")
    @foo = "test"
    @x = x
  end
end
