# typed: strict

module Root::B
  class Child < Root::A::Parent; end

  # Descendants of a tracked class are tracked too, so this must be registered against `Child`.
  class GrandChild < Child; end

  class Includer
    include Root::A::Mixin
  end

  class Unregistered < Root::A::Parent; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add T::Descendants.register(Root::A::Parent, Root::B::Unregistered, "test/testdata/packager/track_descendants/b/b.rb") in an RBI file in this package

  # Registered, but against the wrong superclass.
  class WrongSuperclass < Child; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add T::Descendants.register(Root::B::Child, Root::B::WrongSuperclass, "test/testdata/packager/track_descendants/b/b.rb") in an RBI file in this package

  # Not a descendant of a tracked class, so it doesn't need to be registered.
  class Plain < Root::A::Untracked; end
end

T::Descendants.register(Root::A::Parent, Root::B::Plain, "test/testdata/packager/track_descendants/b/b.rb") # error: `register` can only be called in an RBI file
