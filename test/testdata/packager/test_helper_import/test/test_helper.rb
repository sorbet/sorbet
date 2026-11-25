# typed: strict

module Test::RootPkg
  class ExampleHelper
    A::Thing # allowed

    B::Thing # allowed
    B::PrivateThing
    C::Thing # allowed
  end
end
