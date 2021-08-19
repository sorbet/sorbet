# typed: strict

class A < PackageSpec
  strict_exports true
  export A::B
end
