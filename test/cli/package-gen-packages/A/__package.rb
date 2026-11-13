# typed: strict

class A < PackageSpec
  strict_dependencies 'dag'
  layer 'app'
end
