# typed: strict

class A < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  # Note: NOT exporting A::CONSTANT_FROM_A or A::UnexportedClass
end
