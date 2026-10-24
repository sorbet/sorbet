# typed: strict

class Outer < PackageSpec
  outer_package_with_namespace_behavior!

  layer 'util'
  strict_dependencies 'false'

  export Outer::Thing
end
