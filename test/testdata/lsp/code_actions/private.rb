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
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Use `private` to define private instance methods
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Use `private` to define private instance methods
end

class D
  private def self.foo; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error-with-dupes: Use `private_class_method` to define private class methods
# ^^^^^^^^^^^^^^^^^^^^^^^^^ apply-code-action: [B] Use `private_class_method` to define private class methods
end
