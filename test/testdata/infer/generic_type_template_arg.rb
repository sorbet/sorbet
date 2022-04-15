# typed: true

module ModuleTypeMemberTypeHasTypeParam
  extend T::Generic
  extend T::Sig

  # Corner case for ResolveTypeMembersWalk: The type T.class_of cannot be resolved until A is resolved, but needs
  # to be resolved in order for Y to be resolved.
  Y = type_member(:in) {{upper: T.class_of(A)}}
end

module ModuleTypeAliasTypeHasTypeParams
  extend T::Sig 

  # Same as above.
  Z = T.type_alias{T.class_of(A)}
end

class A
  extend T::Generic
  X = type_template
end

T.reveal_type(T::Array[A].new) # error: Revealed type: `T::Array[A]`
T.reveal_type(T::Array[Integer].new) # error: Revealed type: `T::Array[Integer]`
