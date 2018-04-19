# typed: strict

module NS
  module Inner
  end
end

class A
  N = NS
  N::Inner
end
