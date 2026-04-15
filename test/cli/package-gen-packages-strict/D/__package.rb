# typed: strict

class D < PackageSpec
  layer 'util'
  strict_dependencies 'dag'

  export_all!
end
