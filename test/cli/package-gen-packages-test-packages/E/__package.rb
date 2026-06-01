# typed: strict

class E < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export E::CONSTANT_FROM_E

  visible_to DoesNotExis
  visible_to AlsoDoesNotExist::*
end
