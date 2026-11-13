# typed: strict

class AnotherPackage < PackageSpec
  strict_dependencies 'dag'
  layer 'util'
end
