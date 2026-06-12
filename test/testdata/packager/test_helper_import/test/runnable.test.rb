# typed: strict

module Test::RootPkg
  class RunnableTest
    A::Thing # allowed
    B::Thing # allowed
    C::Thing # allowed
  end
end
