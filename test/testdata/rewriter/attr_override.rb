# typed: strict

class Foo
  extend T::Sig

  sig {returns(T.nilable(Integer))}
  attr_accessor :method1, :method2, :method3, :method4
end

class FooChild < Foo
  extend T::Sig

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
end
