# typed: strict

class D < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import B
  export D::AlreadyExportedClass
end
