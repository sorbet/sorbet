# typed: true

module MyPackage

  # This is a subclass of a sealed parent that is exported only for tests. As a
  # result it will exist in a different rbi file and would normally trigger an
  # error in the definition validator. We opt it out of that check in this case,
  # as we assume the original definition that was used to derive the interface
  # was valid.
  class TestSealedChild < SealedParent
  end

end
