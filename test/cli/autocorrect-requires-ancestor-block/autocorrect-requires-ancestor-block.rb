# typed: true

class RA1; end
module RA2; end
module RA3; end

module M
  extend T::Helpers

  requires_ancestor RA1
  requires_ancestor RA2, RA3
end
