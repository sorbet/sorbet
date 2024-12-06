# frozen_string_literal: true
# typed: strict

class A < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'business'

  import B # error: Strict Dependencies violation: Importing `B` will put `A` into a cycle, which is not valid at `strict_dependencies` level `layered_dag`
  import D # error: Unable to resolve constant `D`
  test_import C
end
