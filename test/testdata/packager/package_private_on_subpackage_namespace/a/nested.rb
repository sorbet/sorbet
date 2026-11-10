# typed: strict

module Root::A
  module Nested
    package_private!
  # ^^^^^^^^^^^^^^^^ error: `package_private!` cannot mark `Root::A::Nested`, because it encloses another package
  end
end
