# typed: strict

extend T::Sig

class Parent; end
class ChildA; end
class ChildB; end

ChildAorChildB = T.type_alias {T.any(ChildA, ChildB)}

sig {returns([Parent])}
def example1
  [ChildAorChildB]
# ^^^^^^^^^^^^^^^^ error: Expected `[Parent]` but found `[<Type: T.any(ChildA, ChildB)>]` for method result type
end

sig {returns([T.untyped])}
def example2
  [ChildAorChildB]
end

sig {returns([Parent, T::Boolean])}
def example3
  [ChildAorChildB, true]
# ^^^^^^^^^^^^^^^^^^^^^^ error: Expected `[Parent, T::Boolean]` but found `[<Type: T.any(ChildA, ChildB)>, TrueClass]` for method result type
end

sig {returns([T.untyped, T::Boolean])}
def example4
  [ChildAorChildB, true]
end

sig {returns([true, false])}
#             ^^^^ error: Unsupported literal in type syntax
#                   ^^^^^ error: Unsupported literal in type syntax
def example5
  [true, false]
end

sig {returns([TrueClass, FalseClass])}
def example6
  [true, false]
end

sig {returns([ChildAorChildB, FalseClass])}
def example7
  [ChildA.new, false]
end
