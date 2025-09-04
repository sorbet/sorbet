# typed: strict

class A < PackageSpec
  strict_dependencies 'dag'
  layer 'app'

  export A::CONSTANT_FROM_A
end
