# typed: strict
# enable-packager: true
# enable-package-directed: true

class Exempt < PackageSpec
  imported_by_nested_packages!
end
