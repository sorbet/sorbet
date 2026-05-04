# typed: strict

class Intermediate < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import Later

  export Intermediate::CONSTANT
end
