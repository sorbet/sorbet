# typed: strict

class A < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  foo 'a'

  export A::CONSTANT_FROM_A

  import Prelude

  test_import B
end
