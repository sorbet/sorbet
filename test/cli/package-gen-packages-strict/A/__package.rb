# typed: strict

class A < PackageSpec
  layer 'app'
  strict_dependencies 'dag'

  foo 'a'

  export A::CONSTANT_FROM_A

  import Prelude

  test_import B
end
