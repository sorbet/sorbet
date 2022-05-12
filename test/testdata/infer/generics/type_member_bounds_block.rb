# typed: true
extend T::Sig

class MyGenericUpper
  extend T::Generic

  Elem = type_member {{upper: Numeric}}
end

class MyGenericLower
  extend T::Generic

  Elem = type_member {{lower: Numeric}}
end

x = MyGenericUpper[String].new
#                  ^^^^^^ error: `String` is not a subtype of upper bound of type member `::MyGenericUpper::Elem`
T.reveal_type(x) # error: `MyGenericUpper[T.untyped]`
x = MyGenericUpper[Integer].new
T.reveal_type(x) # error: `MyGenericUpper[Integer]`

x = MyGenericLower[Integer].new
#                  ^^^^^^^ error: `Integer` is not a supertype of lower bound of type member `::MyGenericLower::Elem`
T.reveal_type(x) # error: `MyGenericLower[T.untyped]`
x = MyGenericLower[Numeric].new
T.reveal_type(x) # error: `MyGenericLower[Numeric]`

