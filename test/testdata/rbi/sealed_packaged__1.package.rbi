# typed: true

module MyPackage

  class SealedParent
    extend T::Helpers

    sealed!
  end

  # This is an exported child of SealedParent, present in the same rbi that
  # defines SealedParent, and as a result passing the sealed checks in
  # definition validator.
  class ExportedChild < SealedParent
  end

end
