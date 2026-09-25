# typed: strict

class A::B < PackageSpec
  # Puts `A::B` in a later stratum than `A::B::C`'s application code.
  import A::B::C
end
