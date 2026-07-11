# typed: strict

module Nested
  module Outer
    module InnerPublic
    end

    module InnerPrivate
      package_private!
    end
  end

  module PrivateOuter
    package_private!

    module InnerThing
    end

    module InnerOtherThing
      package_private!
    end
  end
end
