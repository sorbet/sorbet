# typed: true

module A::B::C
  def foo; end
end

A::B = 1 # error: Cannot initialize the class or module `B` by constant assignment
