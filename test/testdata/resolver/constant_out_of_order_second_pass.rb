# check-out-of-order-constant-references: true
# typed: true

module ResolvedOnFirstPass
end

class A
  include ResolvedOnFirstPass
  include ResolvedOnSecondPass
  #       ^^^^^^^^^^^^^^^^^^^^ error: `ResolvedOnSecondPass` referenced before it is defined
end

module ResolvedOnFirstPass
  module ResolvedOnSecondPass
  end
end

