# typed: strict
# enable-packager: true
# enable-package-directed: true

class SelfImport < PackageSpec
  import SelfImport # error: Package `SelfImport` cannot import itself
end
