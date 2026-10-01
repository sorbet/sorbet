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

  # The unimported package is in a later stratum, so its namespace has not been
  # entered and package-directed mode does not report the ambiguity.
  module UnimportedEmpty::B
  end
end
