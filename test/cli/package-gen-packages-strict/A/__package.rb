# typed: strict

class A < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  foo 'a'

  import Prelude
end
