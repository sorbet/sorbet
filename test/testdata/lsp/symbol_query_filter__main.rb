# typed: true

# This file intentionally contains only a single reference to (or definition of) different types of symbols to test
# the file filtering logic in `setupLSPQueryBySymbol`.

class Foo < AbstractFoo
    # ^ def: Foo
          # ^ usage: AbstractFoo

    CONST = 10
  # ^ def: const

  def isFoo
    # ^ def: isFoo
    true
  end
end

class SomeClass
  include HelperMethods
        # ^ usage: HelperMethods
  extend StaticHelperMethods
       # ^ usage: StaticHelperMethods

  def some_instance_method
    helper_method
  # ^ usage: helper_method
  end
end

SecondClass::FIVE
# ^ usage: SecondClass
           # ^ usage: five

module AModule::SubModule
     # ^ usage: AModule
end

class SomeClass < AnotherModule::NestedModule
                # ^ usage: AnotherModule
  extend AnotherModule2::NestedModule
       # ^ usage: AnotherModule2
  include AnotherModule3::NestedModule
        # ^ usage: AnotherModule3
end
