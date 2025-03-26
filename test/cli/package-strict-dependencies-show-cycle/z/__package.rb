# typed: strict

class Z < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import Y
end
