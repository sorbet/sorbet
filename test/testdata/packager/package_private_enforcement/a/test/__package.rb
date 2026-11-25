# typed: strict

class Root::A::Test < PackageSpec
  test!

  import Root::A, uses_internals: true
end
