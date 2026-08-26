# typed: strict

class A < PackageSpec
  layer 'app'
  strict_dependencies 'dag'

  export_all!
end
