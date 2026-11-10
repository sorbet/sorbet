# typed: strict

module Root::B
  class Child < Root::A::Parent; end

  # Descendants of a tracked class are tracked too, so this must be registered against `Child`.
  class GrandChild < Child; end

  class Includer
    include Root::A::Mixin
  end

  class Unregistered < Root::A::Parent; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add T::Descendants.register(Root::A::Parent, Root::B::Unregistered) in an RBI file in this package

  # Registered, but against the wrong superclass.
  class WrongSuperclass < Child; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add T::Descendants.register(Root::B::Child, Root::B::WrongSuperclass) in an RBI file in this package

  # Not a descendant of a tracked class, so it doesn't need to be registered.
  class Plain < Root::A::Untracked; end
end

T::Descendants.register(Root::A::Parent, Root::B::Plain) # error: `register` can only be called in an RBI file
