# typed: strict

class Y < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import Z
end
