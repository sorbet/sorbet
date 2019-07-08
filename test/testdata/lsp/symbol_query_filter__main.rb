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

class SomeModule
  include HelperMethods
        # ^ usage: HelperMethods

  def some_instance_method
    helper_method
  # ^ usage: helper_method
  end
end

SecondClass::FIVE
           # ^ usage: five
