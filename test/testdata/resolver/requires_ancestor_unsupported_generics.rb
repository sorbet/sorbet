# typed: true
# enable-experimental-requires-ancestor: true

### Requiring generic classes or modules is not supported yet

module Test1
  module RA
    extend T::Generic
    E = type_member
  end

  module M # error: `Test1::M` can't require generic ancestor `Test1::RA` (unsupported)
    extend T::Helpers
    requires_ancestor { RA }
  end
end

module Test2
  class RA
    extend T::Generic
    E = type_member
  end

  module M # error: `Test2::M` can't require generic ancestor `Test2::RA` (unsupported)
    extend T::Helpers
    requires_ancestor { RA }
  end
end

module Test3
  class RA
    class << self
      extend T::Sig
      extend T::Generic

      Elem = type_member

      sig {returns(T.nilable(Elem))}
      attr_accessor :elem

      sig { params(elem: T.nilable(Elem)).void }
      def initialize(elem)
        @elem = T.let(elem, T.nilable(Elem))
      end
    end
  end

  module M # error: `Test3::M` can't require generic ancestor `T.class_of(Test3::RA)` (unsupported)
    extend T::Sig
    extend T::Helpers

    requires_ancestor { T.class_of(RA) }

    sig { void }
    def elem
      self.elem
    end
  end
end

module Test4
  class RA
    extend T::Sig
    extend T::Generic

    Elem = type_template

    sig {returns(T.nilable(Elem))}
    def self.elem
      @elem
    end

      sig { params(elem: T.nilable(Elem)).void }
    def self.initialize(elem)
      @elem = T.let(elem, T.nilable(Elem))
    end
  end

  module M # error: `Test4::M` can't require generic ancestor `T.class_of(Test4::RA)` (unsupported)
    extend T::Sig
    extend T::Helpers

    requires_ancestor { T.class_of(RA) }

    sig { void }
    def elem
      self.elem
    end
  end
end
