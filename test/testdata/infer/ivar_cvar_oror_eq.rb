# typed: true

# Test that Sorbet correctly infers the types of class variables and instance
# variables when they are used as the LHS of ||=.

class C
  extend T::Sig

  @@nilable_var = T.let(nil, T.nilable(Integer))
  @@falsy_var = T.let(false, T.any(FalseClass, Integer))
  @@nevernil = T.let(81, Integer)

  sig {void}
  def initialize
    @ivar = T.let(nil, T.nilable(String))
    @ivar
  end

  sig {params(x: Integer).returns(Integer)}
  def self.f1(x)
    @@nilable_var ||= x
  end

  sig {params(x: Integer).returns(Integer)}
  def self.f2(x)
    @@nevernil ||= x # error: This code is unreachable
  end

  sig {params(x: Integer).returns(Integer)}
  def self.f3(x)
    @@nilable_var ||= x
    @@nilable_var
  end

  sig {params(x: Integer).returns(Integer)}
  def self.f4(x)
    @@falsy_var ||= x
    @@falsy_var
  end

  sig {params(x: Integer).returns(Integer)}
  def self.f5(x)
    @@falsy_var ||= x
  end

  sig {params(x: Integer).returns(Integer)}
  def h1(x)
    @@nilable_var ||= x
  end

  sig {params(x: Integer).returns(Integer)}
  def h2(x)
    @@nevernil ||= x # error: This code is unreachable
  end

  sig {params(x: Integer).returns(Integer)}
  def h3(x)
    @@nilable_var ||= x
    @@nilable_var
  end

  sig {params(x: Integer).returns(Integer)}
  def h4(x)
    @@falsy_var ||= x
    @@falsy_var
  end

  sig {params(x: Integer).returns(Integer)}
  def h5(x)
    @@falsy_var ||= x
  end

  sig {params(x: String).returns(String)}
  def g1(x)
    @ivar ||= x
  end

  sig {params(x: String).returns(String)}
  def g2(x)
    @ivar ||= x
    @ivar
  end
end
