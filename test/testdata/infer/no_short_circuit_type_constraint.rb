# typed: true
extend T::Sig

# This is a comment I wrote to myself while I was working on the original
# change that "fixed" the behavior in this test. It's super verbose, so I don't
# think it belongs in the source code, but I was pretty sure I'd want to be
# able to refer back to it at some point in the future.
#
# It basically walks through how Sorbet's isSubTypeUnderConstraint algorithm
# worked at a point in time, on a handful of the examples in this test.
#
# It shows how the greedy nature of Sorbet's constraint solving algorithm
# (instead of doing proper constraint generation and constraint resolution) can
# break down, so that we don't have enough information to know what we'd like
# to do until later.

# (1)
# T.nilable(Integer) <: T.nilable(T.type_parameter(:T))
#
# ==> NilClass <: T.nilable(T.type_parameter(:T)) &&
#     Integer <: T.nilable(T.type_parameter(:T))
#
# ==> (NilClass <: NilClass ||
#      NilClass <: T.type_parameter(:T)) &&      This should ideally not remember `NilClass` as lower bound
#     (Integer <: NilClass ||
#      Integer <: T.type_parameter(:T))          This should ideally remember `Integer` as lower bound

# (2)
# T.any(Integer, NilClass) <: T.any(T.type_paramater(:T), NilClass)
#
# ==> Integer <: T.any(T.type_parameter(:T), NilClass) &&
#     NilClass <: T.any(T.type_parameter(:T), NilClass)
#
# ==> (Integer <: T.type_parameter(:T) ||        This should ideally remember `Integer` as a lower bound
#      Integer <: NilClass) &&
#     (NilClass <: T.type_parameter(:T) ||       This should ideally not remember `NilClass` as a lower bound
#      NilClass <: NilClass)
#
# Compare this with the previous case. Ideally we want to implement
# isSubTypeUnderConstraint such that it's agnostic of the order of things in a
# union type. To get this test to pass, I did not do that--this test at time of
# writing only works because the order is currently always
# `T.nilable(T.type_parameter(:U))`. This is sort of an accident, because the
# `TypePtr::kind` for `T.type_parameter` is greater than the kind for
# `ClassType`, and we sort the arguments to `lub` on construction based on the
# value of `TypePtr::kind`.

# (3)
# T.any(Integer, NilClass) <: T.nilable(T.type_parameter(:T))
#
# ==> Integer <: T.nilable(T.type_parameter(:T)) &&
#     NilClass <: T.nilable(T.type_parameter(:T))
#
# ==> (Integer <: NilClass ||
#      Integer <: T.type_parameter(:T)) &&
#     (NilClass <: NilClass ||
#      NilClass <: T.type_paramater(:T))

# (4)
# T.untyped <: T.nilable(T.type_parameter(:T))
#
# ==> T.untyped <: NilClass ||                   This is already true, so technically we know enough to
#                                                answer isSubTypeUnderConstraint by short circuiting.
#     T.untyped <: T.type_parameter(:T)          This should also remember `T.untyped` as lower bound

sig do
  type_parameters(:T)
  .params(x: T.nilable(T.type_parameter(:T)))
  .returns(T.type_parameter(:T))
end
def refute_nil(x)
  T.reveal_type(x) # error: `T.nilable(T.type_parameter(:T) (of Object#refute_nil))`
  case x
  when NilClass then raise "was not nil"
  else
    T.reveal_type(x) # error: `T.type_parameter(:T) (of Object#refute_nil)`
    x
  end
end

nilable_integer = T.let(0, T.nilable(Integer))
integer_or_nilclass = T.let(0, T.any(Integer, NilClass))
untyped = T.unsafe(0)

x = refute_nil(nilable_integer)
T.reveal_type(x) # error: `Integer`

x = refute_nil(integer_or_nilclass)
T.reveal_type(x) # error: `Integer`

x = refute_nil(untyped)
T.reveal_type(x) # error: `T.untyped`


sig do
  type_parameters(:T)
  .params(x: T.any(T.type_parameter(:T), NilClass))
  .returns(T.type_parameter(:T))
end
def refute_nil_opposite_order(x)
  T.reveal_type(x) # error: `T.nilable(T.type_parameter(:T) (of Object#refute_nil_opposite_order))`
  case x
  when NilClass then raise "was not nil"
  else
    T.reveal_type(x) # error: `T.type_parameter(:T) (of Object#refute_nil_opposite_order)`
    x
  end
end

x = refute_nil_opposite_order(nilable_integer)
T.reveal_type(x) # error: `Integer`

x = refute_nil_opposite_order(integer_or_nilclass)
T.reveal_type(x) # error: `Integer`

x = refute_nil_opposite_order(untyped)
T.reveal_type(x) # error: `T.untyped`
