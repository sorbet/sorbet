# typed: strict

# Its unreferenced `import Outer::Unused` is what satisfies `Outer`'s requirement, so it's kept.
class Outer::Unused::Inner < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  import Outer::Unused
end
