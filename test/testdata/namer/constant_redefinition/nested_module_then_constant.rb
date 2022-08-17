# typed: true

module A::B::C
  def foo; end
end

A::B = 1 # error: Redefining constant `B` as a static field
