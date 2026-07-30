# typed: strict

class Test::RootPkg < PackageSpec
  test!

  import RootPkg, uses_internals: true
  import A
  import B
  import C
end
