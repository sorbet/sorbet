# typed: strict

class B < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import A
end
