# typed: strict

class F < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import G
  import B
end
