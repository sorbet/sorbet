# typed: strict

class B < PackageSpec
  layer 'util'
  strict_dependencies 'false'

  import D
  import C
  import A

  visible_to A
end
