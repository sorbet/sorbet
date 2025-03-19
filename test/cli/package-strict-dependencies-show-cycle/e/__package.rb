# typed: strict

class E < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import F
end
