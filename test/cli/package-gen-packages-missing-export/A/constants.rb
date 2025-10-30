# typed: strict

module A
  CONSTANT_FROM_A = "Hello from Package A"
  ANOTHER_CONSTANT = "Another constant"

  class UnexportedClass
    def self.hello
      "Hello from UnexportedClass"
    end
  end
end
