# typed: strict

class B < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export B::CONSTANT_FROM_B
end
