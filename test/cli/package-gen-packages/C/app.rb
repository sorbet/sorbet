# typed: strict

module C
  class App
    def self.run
      puts B::CONSTANT_FROM_B
      puts E::CONSTANT_FROM_E
      puts F::CONSTANT_FROM_F
      puts G::CONSTANT_FROM_G

      puts B::CONSTANT_FROM_B
      puts E::CONSTANT_FROM_E

      puts D::CONSTANT_FROM_D
      puts D::ClassFromD
      puts D::ClassFromD::CONSTANT_FROM_D_CLASS
      puts D::EnumFromD::Variant
    end
  end
end
