# typed: strict

class Root::A < PackageSpec
  # Ensure that Root goes first in a package-directed scenario.
  import Root

  export Root::A
end
