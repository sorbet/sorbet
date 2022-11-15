# typed: true

class Parent
  extend T::Sig
  extend T::Generic

  sig {returns(T.attached_class)}
  def self.make
    res = new
    T.reveal_type(res) # error: `T.attached_class (of Parent)`
    res
  end

  sig {params(x: X).void}
  #              ^ error: Unable to resolve constant `X`
  def example(x)
    T.reveal_type(x) # error: `Parent::X (unresolved)`
  end
end

class Child < Parent
end

T.reveal_type(Parent.make) # error: `Parent`
T.reveal_type(Child.make) # error: `Child`
