# typed: true

class Dog
  extend T::Sig

  sig {returns(String)}
  attr_reader :breed
              # ^ sig {returns(String)}
end

def main
  dogs = T.let([], T::Array[Dog])
  # Check hover of breeds / map / : / breed
  breeds = dogs.map(&:breed)
# ^ hover: T::Array[String]
              # ^ hover:     blk: T.proc.params(arg0: Dog).returns(String)
                   # ^ hover: sig { returns(String) }
                    # ^ hover: sig { returns(String) }

  # Safenav
  dog = Dog.new
  breed = dog&.breed
  #          ^^ error: Used `&.` operator on `Dog`, which can never be nil
# ^ hover: String
  #       ^ hover: Dog
  #          ^ hover: Dog
  #           ^ hover: sig { returns(String) }
  #            ^ hover: sig { returns(String) }
  #             ^ hover: sig { returns(String) }

  maybeDog = T.let(nil, T.nilable(Dog))
  maybeBreed = maybeDog&.breed
# ^ hover: T.nilable(String)
  #            ^ hover: T.nilable(Dog)
  #                     ^ hover: NilClass
  #                      ^ hover: sig { returns(String) }
  breed2 = T.let(nil, T.nilable(String))
  breed2 ||= maybeDog&.breed
# ^ hover: T.nilable(String)
  #                   ^ hover: NilClass
  #                    ^ hover: sig { returns(String) }
  breed2 &&= maybeDog&.breed
  # ^ hover: T.nilable(String)
  #                   ^ hover: NilClass
  #                    ^ hover: sig { returns(String) }
end
