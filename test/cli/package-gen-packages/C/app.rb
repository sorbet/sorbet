# typed: strict

module PackageC
  class App
    def self.run
      puts PackageA::CONSTANT_FROM_A
      puts PackageB::CONSTANT_FROM_B
    end
  end
end
