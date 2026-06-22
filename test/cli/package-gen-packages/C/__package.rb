# typed: strict

class C < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import A
  import AnotherPack
  import D
  import PackageWithNoImportsExports

  visible_to DoesNotExist
  visible_to AlsoDoesNotExist::*
end
