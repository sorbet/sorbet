# typed: strict

class C < PackageSpec
  layer 'util'
  strict_dependencies 'dag'

  # strict_dependencies 'false', 'layered', or 'layered_dag':
  import B

  export_all!
end
