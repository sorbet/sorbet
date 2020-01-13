# typed: true

class Module
  include T::Sig
end

class Parent
  sig {returns(T.attached_class)}
  def self.foo
    # Goal of test: no < ... > in the user-visible type.
    T.reveal_type(new) # error: Revealed type: `T.attached_class (of Parent)`
  end
end

class Child < Parent
  sig {returns(T.attached_class)}
  def self.foo
    # Goal of test: no < ... > in the user-visible type.
    T.reveal_type(new) # error: Revealed type: `T.attached_class (of Child)`
  end
end
