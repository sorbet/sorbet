# typed: strict

class Outer::Used < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  import Outer
end
