# typed: strict

class Mid < PackageSpec
  imported_by_nested_packages! # error: Method `imported_by_nested_packages!` does not exist on `T.class_of(Mid)`
end
