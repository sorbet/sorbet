# typed: strict

module RootPkg
  class Example
    A::Thing # allowed
    B::Thing
  # ^ error: `B` is not imported

    C::Thing
  # ^ error: `C` is not imported
  end
end
