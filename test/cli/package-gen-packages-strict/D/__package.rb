# typed: strict

class D < PackageSpec
  layer 'util'
  strict_dependencies 'false'

  import E

  export_all!
end
