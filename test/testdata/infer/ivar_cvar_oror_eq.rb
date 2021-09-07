# typed: true

# Test that Sorbet correctly infers the types of @@var in f and @ivar in g.

class C
  extend T::Sig

  @@var = T.let(nil, T.nilable(Integer))

  def initialize
    @ivar = T.let(nil, T.nilable(String))
  end

  sig {params(x: Integer).returns(Integer)}
  def self.f(x)
    @@var ||= x
  end

  sig {params(x: String).returns(String)}
  def g(x)
    @ivar ||= x
  end
end
