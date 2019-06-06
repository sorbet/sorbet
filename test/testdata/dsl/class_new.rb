# typed: true

class Module
  include T::Sig
end

A = Class.new do
  def foo; end
end

A.new.foo

B = Class.new do
  sig {returns(Integer)}
  def bar
    0
  end
end

T.reveal_type(B.new.bar) # error: Revealed type: `Integer`

module M; end
C = Class.new do
  include M
end

T.let(C.new, M)

D = Class.new do
  sig {void}
  def self.qux; end
end

D.qux

module Top
  class Parent; end
end
E = Class.new(Top::Parent) do; end

T.let(E.new, Top::Parent)


