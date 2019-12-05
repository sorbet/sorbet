# typed: true

# This is a regression test for the ResolveTypeParamsWalk pass of the resolver,
# ensuring that type aliases are tracked as dependencies to other type aliases.

module Foo
  Table = T.type_alias {Types::TABLE}
  Values = T.let([], Table)
end

module Types
  TABLE = T.type_alias {T::Array[Integer]}
end
