# typed: strict

# at 1 because we depend on A but have a cycle with E

# stratum: 1

class E < PackageSpec
  import D
end
