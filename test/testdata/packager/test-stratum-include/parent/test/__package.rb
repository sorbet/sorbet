# typed: strict

# stratum: 2

class Test::Parent < PackageSpec
  test!

  import Child
  import Parent, uses_internals: true
end
