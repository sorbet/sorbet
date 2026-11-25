# typed: strict

class Root::Test < PackageSpec
  test!

  import Root, uses_internals: true

end
