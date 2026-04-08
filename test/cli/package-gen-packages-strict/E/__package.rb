# typed: strict

class E < PackageSpec
  layer 'util'
  strict_dependencies 'layered'

  import D

  export_all!
end
