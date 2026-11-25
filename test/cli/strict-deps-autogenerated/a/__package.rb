# typed: strict

class A < PackageSpec
  layer 'app'
  strict_dependencies 'dag'
end
