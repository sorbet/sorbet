# typed: strict

module Test::RootPkg
  class RunnableTest
    A::Thing # allowed

    B::Thing # allowed---test helper import

    C::Thing # allowed---test-only import
  end
end
