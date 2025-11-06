# typed: strict

class Parent
  extend T::Sig, T::Helpers
  final!

  sig(:final) { returns(T.attached_class) }
  def self.make
    if [true, false].sample
      # expected and desired
      return Parent.new # error: Expected `T.attached_class (of Parent)` but found `Parent`
    else
      # Also works because Child <: Parent and
      # `T.attached_class` == `Parent` in this case
      return Child.new # error: Expected `T.attached_class (of Parent)` but found `Child`
    end
  end
end

class Child < Parent
  #           ^^^^^^ error: `Parent` was declared as final

  # We don't check type member bounds for attached class, because
  # we assume we did it correctly, but it's not correct in this
  # case
  #
  # I'm not really sure how to craft a test case that exhibits a soundness bug
  # due to the lower bound being incompatible with the Parent lower bound, so
  # instead this formulation just proves that the Child's lower bound is
  # getting set to Child.
  #
  # In any case, the "declared as final" error is reported at `typed: false` so
  # we don't really _need_ another error to be reported for the bad bounds--in
  # fact it's lucky that it's silenced because it's less confusing for the user
  # (don't want to leak implementation details if we don't have to).

  final!

  sig(:final) { returns(T.attached_class) }
  def self.make2
    if [true, false].sample
      return Parent.new # error: Expected `T.attached_class (of Child)` but found `Parent` for method result type
    else
      return Child.new # error: Expected `T.attached_class (of Child)` but found `Child` for method result type
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

klass = T.let(Parent, T::Class[T.anything])
klass = T.let(Parent, T::Class[Parent])
klass = T.let(Parent, T.class_of(Parent))

module HasInvariantAttachedClass
  extend T::Sig, T::Generic
  has_attached_class!

  sig { overridable.params(other: T.attached_class).returns(T.attached_class) }
  def example(other)
    if [true, false].sample
      return other
    else
      return ChildFinal1.new # error: Expected `T.attached_class` but found `ChildFinal1` for method result type
    end
  end
end

class ChildFinal1 # error: Type variance mismatch for `T.attached_class`
  extend HasInvariantAttachedClass
  extend T::Sig, T::Helpers
  final!

  sig(:final) { override.params(other: ChildFinal1).returns(ChildFinal1) }
  def self.example(other) # error: does not match return type of overridable
    ChildFinal1.new
  end
end

class ChildFinal2 # error: Type variance mismatch for `T.attached_class`
  extend HasInvariantAttachedClass
  extend T::Helpers
  final!
end

ChildFinal1.example(ChildFinal1.new)
ChildFinal1.example(ChildFinal2.new)
#                   ^^^^^^^^^^^^^^^ error: Expected `ChildFinal1` but found `ChildFinal2` for argument `other`
ChildFinal2.example(ChildFinal1.new)
#                   ^^^^^^^^^^^^^^^ error: Expected `ChildFinal2` but found `ChildFinal1` for argument `other`
ChildFinal2.example(ChildFinal2.new)

has_invariant_child1 = T.let(ChildFinal1, HasInvariantAttachedClass[ChildFinal1])
has_invariant_child1.example(ChildFinal1.new)

has_invariant_anything = T.let(ChildFinal1, HasInvariantAttachedClass[T.anything]) # error: Argument does not have asserted type `HasInvariantAttachedClass[T.anything]`
has_invariant_anything.example(ChildFinal2.new)

_ = T.let(has_invariant_anything, T::Class[T.anything]) # error: Argument does not have asserted type

