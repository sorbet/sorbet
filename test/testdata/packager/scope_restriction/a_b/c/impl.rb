# frozen_string_literal: true
# typed: false

 module A::B
#^^^^^^^^^^^ error: `A::B` belongs to package `A::B`, which package `A::B::C` does not import
  module C
  end
end
