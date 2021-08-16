# typed: strict

class ImportsBothReversed < PackageSpec
  import A::B
  import A
end
