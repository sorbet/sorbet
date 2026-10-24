# typed: strict

# Importing `Outer::Used` (which it references) is enough, so it doesn't need `import Outer`.
class Outer::Used::Inner < PackageSpec
  layer 'app'
  strict_dependencies 'false'
end
