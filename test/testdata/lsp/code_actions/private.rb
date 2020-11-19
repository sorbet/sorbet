# typed: false
# exhaustive-apply-code-action: true

class A
  private def foo; end
end

class B
  private_class_method def self.foo; end
end

class C
  private_class_method def foo; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Use `private` to define private instance methods
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: No method called `foo` exists to be made `private` in `T.class_of(C)`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Replace with `private`
end

class D
  private def self.foo; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Use `private_class_method` to define private class methods
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: No method called `foo` exists to be made `private` in `D`
# ^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [B] Replace with `private_class_method`
end
