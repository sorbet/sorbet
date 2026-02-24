# typed: strict

class Test::Root < PackageSpec
  test!

  import Root, uses_internals: true
end
