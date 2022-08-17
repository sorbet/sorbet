# typed: true

module A::B::C
  def foo; end
end

  A::B = A::D
# ^^^^^^^^^^^ error: Redefining constant `B` as a static field
       # ^^^^ error: Unable to resolve constant `D`
