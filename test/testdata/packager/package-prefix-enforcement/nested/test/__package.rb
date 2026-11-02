# typed: strict

class Test::Root::Nested < PackageSpec
  test!
  import Root::Nested, uses_internals: true
end
