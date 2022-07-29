# typed: true
# disable-fast-path: true
class CovariantNotAllowed
  extend T::Generic
  Elem = type_member(:in) # error: can only have invariant type members
end

class Invalids
  extend T::Generic
  Exp = type_member(1+1) # error: Invalid param, must be a :symbol
  #                 ^^^ error: Expected `Symbol` but found `Integer` for argument `variance`
  Baz = type_member(:baz) # error: Invalid variance kind, only `:out` and `:in` are supported
  Mama = type_member("mama") # error: Invalid param, must be a :symbol
  #                  ^^^^^^ error: Expected `Symbol` but found `String("mama")` for argument `variance`
  One = type_member(1) # error: Invalid param, must be a :symbol
  #                 ^ error: Expected `Symbol` but found `Integer(1)` for argument `variance`
  ArrOne = type_member([1]) # error: Invalid param, must be a :symbol
  #                    ^^^ error: Expected `Symbol` but found `[Integer(1)]` for argument `variance`
  BadArg = type_member {{junk: 1}}
  #                      ^^^^ error: Unknown key `junk` provided in block to `type_member`
  #                            ^ error: Unsupported literal in type syntax
end

module TypeParamDependsOnTypeParam
  extend T::Generic

  # ResolveTypeMembersWalk corner case: The type of this type member depends on a class with a type parameter.
  Test = type_member(:out) {{upper: Parent[Integer]}}
end

module TypeAliasDependsOnTypeParam
  # ResolveTypeMembersWalk corner case: The type of this type alias depends on a class with a type parameter.
  Test = T.type_alias{Parent[Integer]}
end

class Parent
  extend T::Generic
  Elem = type_member
end

class GoodChild < Parent
  extend T::Generic
  Elem = type_member
  My = type_member
end

class BadChild1 < Parent
  extend T::Generic
  My = type_member
  Elem = type_member # error: Type members for `BadChild1` repeated in wrong order
end

class BadChild2 < Parent # error: must be re-declared
  extend T::Generic
  My = type_member
end

class BadChild3 < Parent
  Elem = 3 # error: Type variable `Elem` needs to be declared as `= type_member(SOMETHING)`
end
