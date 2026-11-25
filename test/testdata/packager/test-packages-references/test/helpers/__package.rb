# typed: strict

class Helpers::Test < PackageSpec
  test!

  import Root, uses_internals: true
  #      ^^^^ import: rootpkg

end
