# typed: strict

class A < PackageSpec
  strict_dependencies 'layered_dag'
  layer 'application'

  import B
end
