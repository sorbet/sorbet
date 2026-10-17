# typed: strict

# stratum: 2

class String < PackageSpec # error: Package `String` cannot be defined because `String` is defined in Sorbet's payload
  import Base
end
