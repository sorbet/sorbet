# typed: strict

class Test::Root::Subpackage < PackageSpec
  test!

  import Root::Subpackage, uses_internals: true
end
