# typed: strict

class B < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import C
end
