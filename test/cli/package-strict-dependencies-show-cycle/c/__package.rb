# typed: strict

class C < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import D
end
