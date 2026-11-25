# typed: strict

class String < PackageSpec # error: Package `String` cannot be defined because `String` is defined in Sorbet's payload
end
