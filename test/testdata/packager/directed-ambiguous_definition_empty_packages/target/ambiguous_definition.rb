# typed: true

module Target
  # The imported package is visible in this stratum. Because it has no runtime
  # definitions, its missing namespace is authoritative and not ambiguous.
  module ImportedEmpty::B
  end

  # An explicit definition makes the compact definition unambiguous, even
  # though a package with the same name is in a later stratum.
  module DeclaredEmpty
  end

  module DeclaredEmpty::B
  end

  # Sorbet cannot inspect the contents of this package because it is not
  # imported and is in a later stratum.
  module UnimportedEmpty::B # error: Definition of `B` is possibly ambiguous
  end
end
