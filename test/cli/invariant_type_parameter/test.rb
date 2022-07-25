# typed: true
extend T::Sig

module IBox
  extend T::Generic
  Elem = type_member(:out)
end

class Box
  include IBox
  extend T::Generic
  Elem = type_member
end

module M; end
module N; end

sig do
  type_parameters(:U)
    .params(
      box: Box[T.type_parameter(:U)],
      f: T.proc.params(box: Box[T.type_parameter(:U)]).void,
      blk: T.proc.params(box: Box[T.type_parameter(:U)]).void
    )
    .void
end
def box_example(box, f, &blk)
  yield box
end

sig do
  type_parameters(:U)
    .params(
      box: IBox[T.type_parameter(:U)],
      f: T.proc.params(box: IBox[T.type_parameter(:U)]).void,
      blk: T.proc.params(box: IBox[T.type_parameter(:U)]).void
    )
    .void
end
def ibox_example(box, f, &blk)
  yield box
end

takes_box_string = T.let(->(box) {}, T.proc.params(box: Box[String]).void)
takes_box_n = T.let(->(box) {}, T.proc.params(box: Box[N]).void)
takes_ibox_n = T.let(->(box) {}, T.proc.params(box: IBox[N]).void)
takes_ibox_string = T.let(->(box) {}, T.proc.params(box: IBox[N]).void)

# The error message here is terrible
#   `T.any(Integer, String)` must be a subtype of `T.type_parameter(:U)` which must be a subtype of `T.noreturn`
# because what happens is that checking the first arg places the bounds
#   Integer <: T.type_parameter(:U) (of Object#box_example) <: Integer
# on the constraint, then the second argument places does a Types::lub (T.any)
# on the lower bound, and a Types::glb (T.all) on the upperbound, both with
# `String`, thus producing
#   T.any(Integer, String) <: T.type_parameter(:U) (of Object#box_example) <: T.noreturn
# because `T.all(Integer, String)` collapses to `T.noreturn`
box_example(Box[Integer].new, takes_box_string) do |box|
  # Approximate solution takes the lower bound because `box` has negative polarity
  T.reveal_type(box)
end

# For a similar reason, even if we switch to modules (to avoid the `T.all`
# collapsing to `T.noreturn`), this doesn't escape the fact that the lower
# bound is still `T.any(M, N)`, which is not a subtype of `T.all(M, N)`. So the
# error is slightly better here, but still doesn't fully explain how it arrived
# at this constraint.
box_example(Box[M].new, takes_box_n) do |box|
end

ibox_m = T.let(Box[M].new, IBox[M])
ibox_integer = T.let(Box[Integer].new, IBox[Integer])
# This error is easy to get right, doesn't even involve generics, just
# inheritance, but it's a good test anyways.
box_example(ibox_m, takes_ibox_n) do |box|
  T.reveal_type(box)
end

# Changing to covarint just takes the `equiv` to a one-sided `isSubType`, which
# still doesn't solve, regardless of class or module.
ibox_example(Box[String].new, takes_ibox_n) do |box|
  T.reveal_type(box)
end
ibox_example(ibox_m, takes_ibox_n) do |box|
  T.reveal_type(box)
end
ibox_example(ibox_integer, takes_ibox_string) do |box|
  T.reveal_type(box)
end
