# typed: strict

class Outer::Unused < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  import Outer
end
