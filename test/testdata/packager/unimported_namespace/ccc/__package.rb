# frozen_string_literal: true
# typed: strict
# enable-packager: true

class CCC < PackageSpec
  # No files in this package, so the `CCC` class does not exist in a
  # non-package file (therefore, doesn't exist at runtime)
end
