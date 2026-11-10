# typed: strict
module Root::A::Inner
  class Y; end
  # The class gets the plain name `Y`, so `private_constant :Y` marks the class, not the static field
  Y = 1 # error: Cannot initialize the class `Y` by constant assignment
  private_constant :Y
end
