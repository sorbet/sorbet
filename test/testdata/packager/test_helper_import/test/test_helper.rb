# typed: strict

module Test::RootPkg
  class ExampleHelper
    A::Thing # allowed

    B::Thing # allowed
    B::PrivateThing
  # ^^^^^^^^^^^^^^^ error: `B::PrivateThing` resolves but is not exported from `B`
    C::Thing # allowed
  end
end
