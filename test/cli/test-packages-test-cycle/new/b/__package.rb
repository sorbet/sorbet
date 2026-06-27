# typed: strict

class New::B < PackageSpec
  layer "lib"
  strict_dependencies "dag"
end
