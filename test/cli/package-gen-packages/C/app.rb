# typed: strict

module C
  class App
    def self.run
      puts B::CONSTANT_FROM_B
      puts E::CONSTANT_FROM_E
      puts F::CONSTANT_FROM_F

      puts B::CONSTANT_FROM_B
      puts E::CONSTANT_FROM_E
    end
  end
end
