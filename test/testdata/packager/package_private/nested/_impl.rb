# typed: strict

module Nested
  module Outer
    module InnerPublic
    end

    module InnerPrivate
      package_private!
    end
  end
end
