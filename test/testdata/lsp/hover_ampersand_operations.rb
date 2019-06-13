# typed: true

class Dog
  extend T::Sig

  sig {returns(String)}
  attr_reader :breed
end

def main
  dogs = T.let([], T::Array[Dog])
  # Check hover of breeds / map / : / breed
  breeds = dogs.map(&:breed)
# ^ hover: T::Array[String]
              # ^ hover: sig {params(blk: T.proc.params(arg0: Dog).returns(String)).returns(T::Array[String])}
                   # ^ hover: sig {returns(String)}
                    # ^ hover: sig {returns(String)}

  # Safenav
  dog = Dog.new
  breed = dog&.breed
# ^ hover: T.nilable(String)
            # ^ hover: sig {returns(String)}
             # ^ hover: sig {returns(String)}
end
