# typed: true
extend T::Sig

class NotABox
  extend T::Generic
end

sig {params(x: NotABox[Integer]).void}
#                      ^^^^^^^ error: `NotABox` is not a generic class, but was given type parameters
#                      ^^^^^^^ error: `NotABox` is not a generic class, but was given type parameters
def example1(x)
  T.reveal_type(x) # error: `NotABox[Runtime object representing type: Integer] (unresolved)
end

sig {params(x: NotABox[]).void}
#                     ^^ error: `NotABox` is not a generic class, but was given type parameters
#                     ^^ error: `NotABox` is not a generic class, but was given type parameters
def example2(x)
  T.reveal_type(x) # error: Revealed type: `NotABox[] (unresolved)`
end

class Box
  extend T::Generic

  Elem = type_member
end

sig {params(x: Box[Integer, String]).void}
#                  ^^^^^^^^^^^^^^^ error: Wrong number of type parameters for `Box`. Expected: `1`, got: `2`
#                  ^^^^^^^^^^^^^^^ error: Wrong number of type parameters for `Box`. Expected: `1`, got: `2`
def example3(x)
  T.reveal_type(x) # error: Revealed type: `Box[Integer]`
end

sig {params(x: Box[]).void}
#                 ^^ error: Wrong number of type parameters for `Box`. Expected: `1`, got: `0`
#                 ^^ error: Wrong number of type parameters for `Box`. Expected: `1`, got: `0`
def example4(x)
  T.reveal_type(x) # error: Revealed type: `Box[T.untyped]`
end
