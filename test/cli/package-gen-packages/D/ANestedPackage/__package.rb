# typed: strict

class D::ANestedPackage < PackageSpec
  strict_dependencies 'dag'
  layer 'util'
end
