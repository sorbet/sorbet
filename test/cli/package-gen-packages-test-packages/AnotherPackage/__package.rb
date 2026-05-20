# typed: strict

class AnotherPackage < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export AnotherPackage::CONSTANT
end
