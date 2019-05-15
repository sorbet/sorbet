# typed: true
class Tree
  extend T::Generic
  Elem = type_member
end

Alias = Tree # this was never intended to work by pay-server uses it
T.assert_type!(Tree[Integer].new, Alias[Integer])
T.assert_type!(Alias[Integer].new, Tree[Integer])
T.assert_type!(Alias[Integer].new, Alias[Integer])
