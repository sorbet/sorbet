# typed: strict

class New::A < PackageSpec
  layer "lib"
  strict_dependencies "dag"
end
