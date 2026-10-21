# typed: strict

# A package with no nested packages can be marked, too: the annotation has no effect.
class Lonely < PackageSpec
  imported_by_nested_packages! # error: Method `imported_by_nested_packages!` does not exist on `T.class_of(Lonely)`
end
