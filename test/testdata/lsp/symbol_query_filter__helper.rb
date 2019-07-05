# typed: true

class AbstractFoo
    # ^ def: AbstractFoo
end


class SecondClass
  FIVE = 5
# ^ def: five
end

Foo.new.isFoo
# ^ usage: Foo
      # ^ usage: isFoo