# typed: strict
# enable-packager: true

class Exempt < PackageSpec
  imported_by_nested_packages! # error: Method `imported_by_nested_packages!` does not exist on `T.class_of(Exempt)`
end
