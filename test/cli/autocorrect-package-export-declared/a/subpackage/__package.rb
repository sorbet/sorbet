# typed: strict

class Root::A::Subpackage < PackageSpec
  # Ensure that Root::A package goes first in a package-directed scenario.
  import Root::A

  export Root::A::Subpackage
end
