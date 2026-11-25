# typed: strict

# A package with no nested packages can be marked, too: the annotation has no effect.
class Lonely < PackageSpec
  imported_by_nested_packages!
end
