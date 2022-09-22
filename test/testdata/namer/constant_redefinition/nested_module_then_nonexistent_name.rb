# typed: true

module A::B::C
  def foo; end
end

  A::B = A::D
# ^^^^^^^^^^^ error: Cannot initialize the class or module `B` by constant assignment
       # ^^^^ error: Unable to resolve constant `D`
