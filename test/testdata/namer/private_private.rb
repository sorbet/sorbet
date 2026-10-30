# typed: true

class A
  private_class_method public def foo; end
# ^^^^^^^^^^^^^^^^^^^^ error: Use `private` to define private instance methods
end

class B
  private private_class_method def self.foo; end
# ^^^^^^^ error: Use `private_class_method` to define private class methods
end
