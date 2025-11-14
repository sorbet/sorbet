# typed: strict

class B < PackageSpec
  strict_dependencies 'layered'
  layer 'util'

  import A

  export_all!
end
