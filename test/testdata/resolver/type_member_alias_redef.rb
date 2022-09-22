# typed: true

class TypeAliasWithPuts
  X = puts
  #   ^^^^ error: Expected `Runtime object representing type: Integer` but found `NilClass` for field
  X = T.type_alias {Integer}
end

class TypeMemberWithPuts
  extend T::Generic

  X = puts
  X = type_member # error: Redefining constant `X` as a type member or type template
end

# This used to crash because we would skip entering type members for any
# constants with an explicit scope, which would make the `::X` resolve to the
# `static-field-type-alias ::X` symbol.
#
# (Without the `::X`, one of them gets mangled but namer records the mangled
# and non-mangled names in the tree eagerly, triggering different code paths
# later in resolver.)

class TypeMemberWithTypeAlias
  extend T::Generic
  ::X = type_member
  #     ^^^^^^^^^^^ error: Expected `Runtime object representing type: Integer` but found `T::Types::TypeMember` for field
  ::X = T.type_alias {Integer}
end

class TypeAliasWithTypeMember
  extend T::Generic
  ::Y = T.type_alias {Integer}
  ::Y = type_member
  #     ^^^^^^^^^^^ error: Expected `Runtime object representing type: Integer` but found `T::Types::TypeMember` for field
end
