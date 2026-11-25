# typed: strict
# enable-packager: true
# enable-package-directed: true

class A < PackageSpec
  # Puts `A` in a later stratum than `Nested`.
  import Nested
end
