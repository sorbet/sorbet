# typed: strict

class B < PackageSpec
  layer 'util'
  strict_dependencies 'false'

  import A
  import C

  export B::CONSTANT_FROM_B
  export B::ANOTHER_CONSTANT_FROM_B

  visible_to A
end
