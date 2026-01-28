# typed: strict

class G < PackageSpec
  strict_dependencies 'dag'
  layer 'util'

  export G::CONSTANT_FROM_G

  visible_to A
end
