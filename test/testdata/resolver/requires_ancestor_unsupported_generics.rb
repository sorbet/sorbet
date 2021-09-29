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
