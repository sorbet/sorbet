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
