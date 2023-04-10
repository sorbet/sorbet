# typed: true

class Abstract
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end
end

Abstract.new # error: Attempt to instantiate abstract class `Abstract`

class SubclassOfAbstract < Abstract
  def foo; end
end

SubclassOfAbstract.new

class AbstractWithSingletonNew
  extend T::Helpers
  abstract!

  def self.new; end
end

AbstractWithSingletonNew.new

class SingletonNew
  def self.new; end
end

class AbstractInheritedFromSingletonNew < SingletonNew
  extend T::Helpers
  abstract!
end

AbstractInheritedFromSingletonNew.new

module ModuleNew
  def new; end
end

class Bar
  extend ModuleNew
  extend T::Helpers
  abstract!
end

Bar.new

class CommonRubyPattern
  extend T::Sig

  class A
    extend T::Helpers
    abstract!
  end

  class B < A; end

  sig{ params(a: T.class_of(A)).void }
  def takesA(a)
    a.new
  end

  def assignsA
    a = T.let(B, T.class_of(A))
    a.new
  end
end
