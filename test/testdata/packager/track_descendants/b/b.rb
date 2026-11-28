# typed: strict

module Root::B
  class Child < Root::A::Parent; end

  # Descendants of a tracked class are tracked too, so this must be registered against `Child`.
  class GrandChild < Child; end

  class Includer
    include Root::A::Mixin
  end

  # Descendants of tracked modules are tracked too, through both subclasses and modules.
  class GrandIncluder < Includer; end

  module SubMixin
    include Root::A::Mixin
  end

  class IndirectIncluder
    include SubMixin
  end

  module DeepMixin
    include SubMixin
  end

  # Includes `Root::A::Mixin` through a chain of modules.
  class DeepIncluder
    include DeepMixin
  end

  # Also includes `Root::A::Mixin` through `SubMixin`, so it only needs to be registered against `SubMixin`.
  class BothIncluder
    include Root::A::Mixin
    include SubMixin
  end

  class Unregistered < Root::A::Parent; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add `Root::B::Unregistered => "test/testdata/packager/track_descendants/b/b.rb"` to T::Descendants.register(Root::A::Parent, ...) in `__descendants.rbi` in this package

  # Registered, but against the wrong superclass.
  class WrongSuperclass < Child; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add `Root::B::WrongSuperclass => "test/testdata/packager/track_descendants/b/b.rb"` to T::Descendants.register(Root::B::Child, ...) in `__descendants.rbi` in this package

  # Not a descendant of a tracked class, so it doesn't need to be registered.
  class Plain < Root::A::Untracked; end
end

T::Descendants.register(Root::A::Untracked, {Root::B::Plain => "test/testdata/packager/track_descendants/b/b.rb"}) # error: `register` can only be called in a file named `__descendants.rbi`
