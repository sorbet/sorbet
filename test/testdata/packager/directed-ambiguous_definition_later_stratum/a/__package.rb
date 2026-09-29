# typed: strict
# enable-packager: true
# enable-package-directed: true

class A < PackageSpec
  # Puts `A` in a later stratum than `Opus::A::B::C`.
  import Opus::A::B::C
end
