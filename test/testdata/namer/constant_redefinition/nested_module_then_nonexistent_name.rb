# typed: true

module A::B::C
  def foo; end
end

  A::B = A::D
# ^^^^^^^^^^^ error: Redefining constant `B`
       # ^^^^ error: Unable to resolve constant `D`
