# typed: strict

class Outer < PackageSpec
  imported_by_nested_packages!

  layer 'util'
  strict_dependencies 'false'
end
