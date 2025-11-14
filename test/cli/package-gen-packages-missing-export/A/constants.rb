# typed: strict

module A
  CONSTANT_FROM_A = "Hello from Package A"

  class UnexportedClass
    ANOTHER_CONSTANT = "Another constant"
    def self.hello
      "Hello from UnexportedClass"
    end
  end
end
