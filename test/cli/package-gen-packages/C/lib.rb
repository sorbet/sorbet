# typed: strict

module C
  class Lib
    def self.run
      puts B::CONSTANT_FROM_B
      puts E::CONSTANT_FROM_E
    end
  end
end
