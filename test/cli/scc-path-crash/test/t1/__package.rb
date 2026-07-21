# typed: strict

class Test::T1 < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'application'

  import Test::T2
end
