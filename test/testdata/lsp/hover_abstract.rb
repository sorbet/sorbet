# typed: true
class AbstractItem
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(String)}
  def self.name; end
    # ^ hover: sig { abstract.returns(String) }
end

class Dog < AbstractItem
  extend T::Sig

  sig {override.returns(String)}
  def self.name
    # ^ hover: sig { override.returns(String) }
    'Dog'
  end
end

module MyInterface
  extend T::Sig
  extend T::Helpers
  interface!

  # Some docstring explaining how to use the method
  sig {abstract.params(x: Integer).returns(String)}
  def frob(x); end
end

class MyImplementation
  extend T::Sig
  include MyInterface

  sig {override.params(x: Integer).returns(String)}
  def frob(x)
    # ^ hover: Some docstring explaining how to use the method
    "implemented"
  end
end
