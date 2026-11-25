# typed: strict

class PackageWithIgnore < PackageSpec
  strict_dependencies 'false'
  layer 'util'
end
