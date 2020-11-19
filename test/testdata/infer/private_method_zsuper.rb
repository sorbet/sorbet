# typed: strict

class Parent
  extend T::Sig

  sig {returns(Integer)}
  def self.foo
    0
  end
end

class Child < Parent
  private_class_method :foo

  T.reveal_type(foo) # error: Revealed type: `Integer`
end

Child.foo # error: Non-private call to private method `foo`
