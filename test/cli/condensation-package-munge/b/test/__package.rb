# typed: strict

class B::Test < PackageSpec
  test!
  import A
  import B
end
