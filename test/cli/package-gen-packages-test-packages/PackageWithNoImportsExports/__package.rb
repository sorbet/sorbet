# typed: strict

class PackageWithNoImportsExports < PackageSpec
  layer 'util'
  strict_dependencies 'dag'
end
