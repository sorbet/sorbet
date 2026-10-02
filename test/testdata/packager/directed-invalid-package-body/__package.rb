# typed: strict
# enable-packager: true
# enable-package-directed: true

class InvalidBody < PackageSpec
  class Nope; end # error: Invalid expression in package: `ClassDef` not allowed
end
