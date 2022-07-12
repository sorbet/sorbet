# typed: true

class A
  # TODO(jez) We should sort exact prefix matches ahead of fuzzy matches
  extend T::S # error-with-dupes: Unable to resolve
  #          ^ completion: ImmutableStruct, InexactStruct, Set, Sig, Struct
  extend T::H # error-with-dupes: Unable to resolve
  #          ^ completion: Hash, Helpers
  extend T::G # error-with-dupes: Unable to resolve
  #          ^ completion: Generic
end
