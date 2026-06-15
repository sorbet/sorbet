# typed: strict

class Test::A < PackageSpec
  test!

  import A
  import B
  import Test::B
end
