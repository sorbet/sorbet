# typed: true

class Parent
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {returns(Elem)}
  attr_accessor :elem

  sig {returns(T.self_type)}
  def get_self; self; end
end

class Child < Parent
  Elem = type_member(fixed: Integer)

  private :elem
  private :get_self

  def example
    T.reveal_type(elem) # error: Revealed type: `Integer`

    self.elem = '' # error: Assigning a value to `elem` that does not match expected type `Integer`

    T.reveal_type(get_self) # error: Revealed type: `Child`
  end
end
