# typed: strict

extend T::Sig

class Parent; end
class ChildA; end
class ChildB; end

ChildAorChildB = T.type_alias {T.any(ChildA, ChildB)}

sig {returns([Parent])}
def example1
  [ChildAorChildB]
# ^^^^^^^^^^^^^^^^ error: Expected `[Parent]` but found `[Runtime object representing type: T.any(ChildA, ChildB)]` for method result type
end

sig {returns([T.untyped])}
def example2
  [ChildAorChildB]
end

sig {returns([Parent, T::Boolean])}
def example3
  [ChildAorChildB, true]
# ^^^^^^^^^^^^^^^^^^^^^^ error: Expected `[Parent, T::Boolean]` but found `[Runtime object representing type: T.any(ChildA, ChildB), TrueClass]` for method result type
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

sig {returns([ChildA, FalseClass])}
def example8
  [ChildA, false] # error: Expected `[ChildA, FalseClass]` but found `[T.class_of(ChildA), FalseClass]` for method result type
end

sig {returns([ChildA, FalseClass])}
def example9
  [ChildA, false] # error: Expected `[ChildA, FalseClass]` but found `[T.class_of(ChildA), FalseClass]` for method result type
end

x1 = T.let(T.unsafe(nil), [true]) # error: Unsupported literal in type syntax
T.reveal_type(x1) # error: Revealed type: `[TrueClass] (1-tuple)`

x2 = T.let(T.unsafe(nil), [TrueClass])
T.reveal_type(x2) # error: Revealed type: `[TrueClass] (1-tuple)`

x3 = T.let(T.unsafe(nil), [ChildA])
T.reveal_type(x3) # error: Revealed type: `[ChildA] (1-tuple)`

x4 = T.let(T.unsafe(nil), [ChildAorChildB])
T.reveal_type(x4) # error: Revealed type: `[T.any(ChildA, ChildB)] (1-tuple)`
