# typed: strict

class D < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export D::CONSTANT_FROM_D
end
