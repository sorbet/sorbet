# typed: strict

class Target < PackageSpec
  import Lib::Base
  test_import TestSupport
  export Target::Value
end
