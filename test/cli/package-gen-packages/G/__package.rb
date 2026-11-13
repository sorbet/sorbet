# typed: strict

class G < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  import B

  visible_to A
end
