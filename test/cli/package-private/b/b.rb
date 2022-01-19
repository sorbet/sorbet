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
end

