# typed: strict

module RootPkg
  class Example
    A::Thing # allowed
    B::Thing
  # ^^^^^^^^ error: `B::Thing` resolves but its package is not imported

    C::Thing
  # ^^^^^^^^ error: `C::Thing` resolves but its package is not imported
  end
end
