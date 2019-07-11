# typed: true

class AbstractFoo
    # ^ def: AbstractFoo
end


class SecondClass
    # ^ def: SecondClass
  FIVE = 5
# ^ def: five
end

module HelperMethods
     # ^ def: HelperMethods
  def helper_method
    # ^ def: helper_method
    3
  end
end

module StaticHelperMethods
     # ^ def: StaticHelperMethods
end

Foo.new.isFoo
# ^ usage: Foo
      # ^ usage: isFoo

module AModule
     # ^ def: AModule
end

# Repeated so each is used once.
module AnotherModule
     # ^ def: AnotherModule
  module NestedModule
  end
end

module AnotherModule2
     # ^ def: AnotherModule2
  module NestedModule
  end
end

module AnotherModule3
     # ^ def: AnotherModule3
  module NestedModule
  end
end

class WillGetAliased
    # ^ def: WillGetAliased
  extend T::Sig

  sig {params(a: AliasForClass).returns(AliasForClass)}
                # ^ usage: AliasForClass
                                      # ^ usage: AliasForClass
  def foo(a)
    a
  end
end

class DefinesInstanceAndClassVars
  @@some_class_var = T.let("h", String)
  # ^ def: some_class_var
  def initialize
    @some_ivar = T.let(10, Integer)
   # ^ def: some_ivar
  end
end
