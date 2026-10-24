# typed: strict

class Outer::Missing < PackageSpec
  layer 'app'
  strict_dependencies 'false'
end
