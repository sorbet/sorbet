# typed: strict
# enable-packager: true
# enable-package-directed: true

class BadTest < PackageSpec
  test! # error: `test!` is only valid for packages with `/test/` in their path
end
