# typed: true
class Animal
  extend T::Sig

  sig {overridable.returns(String)}
  def self.name; "Animal"; end
    # ^ hover: sig { overridable.returns(String) }
end

class Dog < Animal
  extend T::Sig

  sig {override.returns(String)}
  def self.name
    # ^ hover: sig { override.returns(String) }
    'Dog'
  end
end
