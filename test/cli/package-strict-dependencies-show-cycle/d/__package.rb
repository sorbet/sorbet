# typed: strict

class D < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import E
end
