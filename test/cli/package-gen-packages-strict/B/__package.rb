# typed: strict

class B < PackageSpec
  layer 'util'
  strict_dependencies 'false'

  import D
  import C
  import A

  export B::CONSTANT_FROM_B

  visible_to A
end
