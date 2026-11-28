# typed: strict

# Descendants of tracked classes must be tracked regardless of the order in which they're defined. Here, each class is
# defined before its superclass.
module Root::B
  class OutOfOrderGrandChild < OutOfOrderChild; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: add `Root::B::OutOfOrderGrandChild => "test/testdata/packager/track_descendants/b/out_of_order.rb"` to T::Descendants.register(Root::B::OutOfOrderChild, ...) in an RBI file in this package

  class OutOfOrderChild < OutOfOrderParent; end

  class OutOfOrderParent < Root::A::Parent; end

  # Same, but through modules.
  class OutOfOrderIncluder
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: add `Root::B::OutOfOrderIncluder => "test/testdata/packager/track_descendants/b/out_of_order.rb"` to T::Descendants.register(Root::B::OutOfOrderDeepMixin, ...) in an RBI file in this package
    include OutOfOrderDeepMixin
  end

  module OutOfOrderDeepMixin
    include OutOfOrderSubMixin
  end

  module OutOfOrderSubMixin
    include Root::A::Mixin
  end
end
