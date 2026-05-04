# typed: strict

class Later < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export Later::CONSTANT
end
