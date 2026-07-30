# typed: strict

class Test::Helpers < PackageSpec
  test!

  import Root, uses_internals: true
  #      ^^^^ import: rootpkg

end
