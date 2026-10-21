# typed: strict

class Unsatisfied < PackageSpec
  imported_by_nested_packages! # error: Method `imported_by_nested_packages!` does not exist on `T.class_of(Unsatisfied)`
end
