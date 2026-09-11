# frozen_string_literal: true
# typed: false

 module A::B
#^^^^^^^^^^^ error: `A::B` belongs to package `A::B`
  module C
  end
end
