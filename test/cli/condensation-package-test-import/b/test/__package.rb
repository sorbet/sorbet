# typed: strict

class Test::B < PackageSpec
  test!

  import B
  import C
end
