# typed: true

class A
  def self.foo; 1; end
end
class A
  extend T::Sig
  sig {returns(Integer)}
  def self.foo; 2; end
end
T.reveal_type(A.foo) # error: Revealed type: `Integer`

class B
  extend T::Sig
  sig {returns(Integer)}
  def self.foo; 3; end
end
class B
  def self.foo; 4; end
end
T.reveal_type(B.foo) # error: Revealed type: `Integer`
