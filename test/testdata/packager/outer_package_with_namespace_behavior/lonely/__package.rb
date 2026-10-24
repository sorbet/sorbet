# typed: strict

# A package with no nested packages can be marked, too: the annotation has no effect.
class Lonely < PackageSpec
  outer_package_with_namespace_behavior!
end
