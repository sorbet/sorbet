# typed: strict

class E < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  visible_to DoesNotExis
  visible_to AlsoDoesNotExist::*
end
