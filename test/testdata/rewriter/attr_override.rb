# typed: strict

class Foo
  extend T::Sig

  sig {returns(T.nilable(Integer))}
  attr_accessor :method1, :method2, :method3, :method4
end

class FooChild < Foo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {void}
  def initialize
    @method1 = @method2 = @method3 = @method4 = T.let(nil, T.nilable(Integer))
  end

  sig {override.returns(T.nilable(Integer))}
  attr_accessor :method1

  sig {returns(T.nilable(Integer)).override}
  attr_accessor :method2

  sig {override.returns(T.nilable(Integer)).checked(:always)}
  attr_accessor :method3

  sig {returns(T.nilable(Integer)).override.checked(:always)}
  attr_accessor :method4

  sig {returns(T.nilable(Integer)).overridable}
  attr_accessor :method5

  sig {returns(T.nilable(Integer)).abstract}
  def method6; end
end

class FooGrandChild < FooChild
  extend T::Sig

  sig {override.returns(T.nilable(Integer))}
  def method6
    42
  end
end
