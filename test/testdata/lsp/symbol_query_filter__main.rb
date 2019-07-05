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

SecondClass::FIVE
           # ^ usage: five
