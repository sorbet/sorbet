# typed: true

class A
  # TODO(jez) We should sort exact prefix matches ahead of fuzzy matches
  extend T::S # error: Unable to resolve
       # ^^^^ error: Unable to resolve
  #          ^ completion: ImmutableStruct, InexactStruct, Set, Sig, Struct
  extend T::H # error: Unable to resolve
       # ^^^^ error: Unable to resolve
  #          ^ completion: Hash, Helpers
  extend T::G # error: Unable to resolve
       # ^^^^ error: Unable to resolve
  #          ^ completion: Generic
end
