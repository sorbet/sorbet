# typed: strict

class C < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import A
  import AnotherPack
  import D

  visible_to DoesNotExist
  visible_to AlsoDoesNotExist::*
end
