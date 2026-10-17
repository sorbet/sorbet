# typed: strict

# Stratum 2 because we depend on B, at 1
# stratum: 3

class C < PackageSpec
  import B
end
