# typed: strict

class Prelude < PackageSpec
  prelude!

  layer 'app'
  strict_dependencies 'dag'
end
