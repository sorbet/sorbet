# typed: strict
extend T::Sig

# These test cases are a little contrived. To make them easier to think about
# when writing, I wrote diagrams using something like Haskell syntax and
# diagramed the polarities of the type variables.
#
# '+' means "in positive position"
# '-' means "in negative position"

# In this example, the constraint fails to solve in the dispatchCall while
# processing arguments (before we even get to considering the method's block or
# result type).
#
#      +          -
#     (u -> ()) × u -> ()
#
sig do
  type_parameters(:U)
    .params(
      f: T.proc.params(x: T.type_parameter(:U)).void,
      x: T.type_parameter(:U)
    )
    .void
end
def dispatch_call_fail(f, x)
end

f = T.let(
  proc {|x| nil},
  T.proc.params(x: String).void
)

# These constraints must be true:
#
#   T.type_parameter(:U) <: String
#   Integer <: T.type_parameter(:U)
#
# but they can't be.
dispatch_call_fail(f, 0) # error: Could not find valid instantiation of type parameters



# In this example, the constraint only solves once we reach the
# `SolveConstraint` instruction to reconcile the block's return type with the
# constraints we gathered from the non-block arguments.
#
#      +                 -
#     (u -> ()) × (() -> u) -> ()
#
sig do
  type_parameters(:U)
    .params(
      f: T.proc.params(x: T.type_parameter(:U)).void,
      blk: T.proc.returns(T.type_parameter(:U)),
    )
    .void
end
def solve_constraint_fail(f, &blk)
end

# These constraints must be true:
#
#   T.type_parameter(:U) <: String
#   Integer <: T.type_parameter(:U)
#
# but they can't be.
solve_constraint_fail(f) do # error: Could not find valid instantiation of type parameters for `Object#solve_constraint_fail`
  0
end
