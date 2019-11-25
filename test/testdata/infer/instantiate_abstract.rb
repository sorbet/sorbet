# typed: true

class Abstract
  extend T::Helpers
  extend T::Sig
  abstract!

  sig {abstract.void}
  def foo; end
end

class SubclassOfAbstract < Abstract
  def foo; end
end

module B
  extend T::Helpers
  abstract!

  def self.new
  end
end

Abstract.new # error: Attempt to instantiate abstract class `Abstract`
SubclassOfAbstract.new
B.new
