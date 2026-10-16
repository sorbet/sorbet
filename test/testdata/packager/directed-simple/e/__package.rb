# typed: strict

# at 1 because we depend on A but have a cycle with E

# stratum: 2

class E < PackageSpec
  import D
end
