# typed: strict

class F < PackageSpec
  strict_dependencies 'layered'
  layer 'util'

  export F::CONSTANT_FROM_F
end
