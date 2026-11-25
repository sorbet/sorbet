# typed: strict

class B < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  visible_to A
  visible_to G
end
