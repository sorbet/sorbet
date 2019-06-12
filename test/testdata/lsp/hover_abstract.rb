# typed: true
class AbstractItem
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(String)}
  def self.name; end
    # ^ hover: sig {abstract.returns(String)}
end

class Dog < AbstractItem
  extend T::Sig

  sig {implementation.returns(String)}
  def self.name
    # ^ hover: sig {implementation.returns(String)}
    'Dog'
  end
end
