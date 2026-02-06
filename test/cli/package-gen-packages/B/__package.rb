# typed: strict

class B < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export B::CONSTANT_FROM_B

  visible_to A
  visible_to G
end
