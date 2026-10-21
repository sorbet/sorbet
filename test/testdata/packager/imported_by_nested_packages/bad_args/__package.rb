# typed: strict

class BadArgs < PackageSpec
  imported_by_nested_packages!(:arg) # error: Method `imported_by_nested_packages!` does not exist on `T.class_of(BadArgs)`
end
