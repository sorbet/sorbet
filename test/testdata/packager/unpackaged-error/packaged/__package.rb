# frozen_string_literal: true
# typed: strict

# This should not have an unpackaged error since it is a package file.

class MyPackage < PackageSpec # error: Package `MyPackage` is missing imports
  # No imports or exports.
  import OtherPackageImported
end
