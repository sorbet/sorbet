# typed: strict

# Stratum 1 because we depend on A, at 0

# stratum: 1

class B < PackageSpec
  import A
end
