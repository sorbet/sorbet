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

#################

class Foo
  def self.foo; end
end

C1 = Class.new do
  T.reveal_type self # error: Revealed type: `T.class_of(C1)`
end

C2 = Class.new(Foo) do
  T.reveal_type self # error: Revealed type: `T.class_of(C2)`
  foo
end

c1 = Class.new do
  T.reveal_type self # error: Revealed type: `T::Class[T.anything]`
end

c2 = Class.new(Foo) do
  T.reveal_type self # error: Revealed type: `T.class_of(Foo)`
  foo
end

Class.new do
  T.reveal_type(self) # error: Revealed type: `T::Class[T.anything]`
end

Class.new(Foo) do
  T.reveal_type(self) # error: Revealed type: `T.class_of(Foo)`
  foo
end

class Bar
  def bar
    Class.new(Foo) do
      T.reveal_type(self) # error: Revealed type: `T.class_of(Foo)`
    end
  end

  def self.bar
    Class.new(Foo) do
      T.reveal_type(self) # error: Revealed type: `T.class_of(Foo)`
    end
  end
end
