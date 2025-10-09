# typed: strict

class C < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import A
  import D
end
