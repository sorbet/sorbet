# typed: strict

module B
  class App
    def self.run
      # Try to use unexported constant
      puts A::CONSTANT_FROM_A

      # Try to use another unexported constant
      puts A::UnexportedClass::ANOTHER_CONSTANT

      # Try to use unexported class
      puts A::UnexportedClass.hello
    end
  end
end
