# typed: strict

class B
  extend T::Sig

  sig {returns(String)}
  def self.bar
    A.foo
  end
end

