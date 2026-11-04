# typed: strict

# at 1 because we depend on A but have a cycle with E

# stratum: 1

class D < PackageSpec
  import A
  import E
end
