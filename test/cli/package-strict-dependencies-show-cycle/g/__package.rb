# typed: strict

class G < PackageSpec
  strict_dependencies 'layered'
  layer 'application'

  import A
end
