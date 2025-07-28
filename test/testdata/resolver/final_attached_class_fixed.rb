# typed: strict

class Parent
  extend T::Sig, T::Helpers
  final!

  sig(:final) { returns(T.attached_class) }
  def self.make
    if [true, false].sample
      # expected and desired
      Parent.new
    else
      # Also works because Child <: Parent and
      # `T.attached_class` == `Parent` in this case
      Child.new
    end
  end
end

class Child < Parent
  #           ^^^^^^ error: `Parent` was declared as final
  # We don't check type member bounds for attached class, because
  # we assume we did it correctly, but it's not correct in this
  # case
  #
  # I'm not really sure how to craft a test case that exhibits a soundness bug due to the lower bound being incompatible with the Parent lower bound, so instead this formulation just proves that the Child's lower bound is getting set to Child.
  #
  # In any case, the "declared as final" error is reported at #
  # `typed: false` so we don't really _need_ another error to be
  # reported for the bad bounds--in fact it's lucky that it's
  # silenced because it's less confusing for the user (don't want
  # to leak implementation details if we don't have to).

  final!

  sig(:final) { returns(T.attached_class) }
  def self.make2
    if [true, false].sample
      return Parent.new # error: Expected `T.attached_class (of Child)` but found `Parent` for method result type
    else
      return Child.new
    end
  end
end

# Covariant type members default to their upper bound
res = Parent.make
T.reveal_type(res) # error: `Parent`
res = Child.make
T.reveal_type(res) # error: `Child`
res = Child.make2
T.reveal_type(res) # error: `Child`
