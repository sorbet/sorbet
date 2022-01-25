# typed: strict

class B
  extend T::Sig

  sig {returns(String)}
  def self.bar
    A.foo
    A.new.foo

    f = A.new
    f.foo
  end

  module C; end

  sig {params(c_and_a: T.all(C, A)).void}
  def self.qux(c_and_a)
    c_and_a.foo
  end
end

