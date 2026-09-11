# frozen_string_literal: true
# typed: false

# As neither of these reference constants, it should be fine to forward-declare them.
module A
  module B
  end
end

module A::B # error: `A::B` belongs to package `A::B`
  module C
  end
end
