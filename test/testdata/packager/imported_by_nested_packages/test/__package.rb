# typed: strict

class Outer::Test < PackageSpec
  test!

  import Outer, uses_internals: true
end
