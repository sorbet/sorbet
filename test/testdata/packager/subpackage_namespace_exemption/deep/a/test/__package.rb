# typed: strict

class Deep::A::Test < PackageSpec
  test!

  import Deep::A, uses_internals: true
end
