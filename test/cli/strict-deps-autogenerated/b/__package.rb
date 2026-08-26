# typed: strict

class B < PackageSpec
  layer 'app'
  strict_dependencies 'dag'

  import A

  export_all!
end
