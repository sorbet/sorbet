# typed: strict

class Target::Test < PackageSpec
  test!
  import Target
  import TestSupport
end
