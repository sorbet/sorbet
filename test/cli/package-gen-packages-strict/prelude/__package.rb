# typed: strict

class Prelude < PackageSpec
  prelude_package

  layer 'app'
  strict_dependencies 'dag'
end
