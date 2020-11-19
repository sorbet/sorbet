# typed: strict

# This test would technically fail if run in Ruby directly, but this is more or
# less representative of a situation where constants are autoloaded and these
# class defs are in separate files.

class Child < Parent
  private_class_method :foo

  T.reveal_type(foo) # error: Revealed type: `Integer`
end

class Parent
  extend T::Sig

  sig {returns(Integer)}
  def self.foo
    0
  end
end

Child.foo # error: Non-private call to private method `foo`
