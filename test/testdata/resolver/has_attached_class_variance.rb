# typed: true
# disable-fast-path: true

module M
  extend T::Sig, T::Generic
  has_attached_class!

  sig { returns(T.nilable(T.attached_class)) }
  def inst
    @inst
  end

  sig { params(inst: T.attached_class).void }
  def inst=(inst)
    @inst = T.let(inst, T.nilable(T.attached_class))
  end
end

class Parent # error: Type variance mismatch for `T.attached_class` with parent `M`. Parent needs to be `:out` but it is `invariant`
  extend M
  # if this class were final, then Sorbet could allow the invariant <AttachedClass> above
  # final!
end

class Child < Parent
  def foo; end
end

child_class = Child
upcasted_to_parent = T.let(child_class, T.class_of(Parent))
upcasted_to_parent.inst = Parent.new
inst_from_child = child_class.inst
T.reveal_type(inst_from_child) # error: `T.nilable(Child)`
# 💥 at runtime, this holds an instance of `Parent`, not `Child`
