# typed: true

class RA1; end
module RA2; end
module RA3; end
module RA4; end
module RA5; end

module M
  extend T::Helpers

  requires_ancestor RA1
  requires_ancestor RA2, RA3
  requires_ancestor(RA4) { RA5 }
end
