# typed: true

class Dog
  extend T::Sig

  sig {returns(String)}
  attr_reader :name
# ^ hover: sig { returns(String) }

  sig {params(name: String).void}
  def initialize(name)
    @name = T.let(name, String)
  # ^ hover: String
  end

  sig {params(plusDog: Dog).returns(Dog)}
  def +(plusDog)
    Dog.new(@name + plusDog.name)
  end

  sig {params(ltDog: Dog).returns(T::Boolean)}
  def <(ltDog)
    true
  end

  sig {params(eqDog: Dog).returns(T::Boolean)}
  def ==(eqDog)
    @name == eqDog.name
  end
end

def main
  fred = Dog.new('fred')
  di = Dog.new('di')
  freddi = fred + di
              # ^ sig {params(plusDog: Dog).returns(Dog)}

  fred < di
     # ^ sig {params(ltDog: Dog).returns(Boolean)}

  fred == di
     # ^ sig {params(eqDog: Dog).returns(Boolean)}
end
