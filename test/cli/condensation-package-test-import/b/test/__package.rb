# typed: strict

class Test::B < PackageSpec
  test!

  import B
  import C

  export Test::B::Helper
end
