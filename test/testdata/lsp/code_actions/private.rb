# typed: false
# selective-apply-code-action: quickfix

class A
  private def foo; end
end

class B
  private_class_method def self.foo; end
end

class C
  private_class_method def foo; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Use `private` to define private instance methods
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Replace with `private`
end

class D
  private def self.foo; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Use `private_class_method` to define private class methods
# ^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [B] Replace with `private_class_method`
end
