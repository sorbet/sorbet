# typed: strict

# Stratum 1 because we depend on A, at 0

# stratum: 2

class B < PackageSpec
  import A
end
