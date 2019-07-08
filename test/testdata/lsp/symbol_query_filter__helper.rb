# typed: true

class AbstractFoo
    # ^ def: AbstractFoo
end


class SecondClass
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
