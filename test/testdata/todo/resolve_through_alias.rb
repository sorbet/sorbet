# @typed

module NS
  module Inner
  end
end

class A
  N = NS
  N::Inner # error: Stubbing out unknown constant
end
