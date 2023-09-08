# typed: true
extend T::Sig

# A previous version of Sorbet's support for sealed_subclasses manually created
# `ClassType`'s for the elements of the sealed subclasses, instead of
# AppliedTypes, like will normally be created if someone writes a
# `T.class_of(...)` type directly. That meant that once upon a time, the final
# `T.let` below failed to check.
#
# This was particularly pernicious to diagnose, because we print ClassTypes and
# AppliedTypes the same way for singleton classes, so it wasn't obvious why the
# two cases were different. This also means that it's important to use `T.let`
# in this test, because using `T.reveal_type` would show the same type in both
# cases.

class Parent
  extend T::Helpers
  abstract!
  sealed!
end

class Child1 < Parent
end

class Child2 < Parent
end

sig {params(x: T.any(T.class_of(Child1), T.class_of(Child2))).void}
def foo(x)
  T.let(x, T.class_of(Parent))
end

y = T.must(Parent.sealed_subclasses.first)
T.let(y, T.class_of(Parent))

Parent.sealed_subclasses.each do |klass|
  T.reveal_type(klass) # error: `T.any(T.class_of(Child1), T.class_of(Child2))`
  instance = klass.new
  T.reveal_type(instance) # error: `T.any(Child1, Child2)`
end
