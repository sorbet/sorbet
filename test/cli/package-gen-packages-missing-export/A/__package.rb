# typed: strict

class A < PackageSpec
  strict_dependencies 'layered'
  layer 'util'

  # Note: NOT exporting A::CONSTANT_FROM_A or A::UnexportedClass
end
