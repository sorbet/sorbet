# typed: strict

class A::B < PackageSpec
  strict_exports true
  export A::B::C
end
