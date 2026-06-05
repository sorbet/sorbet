# typed: true
module Parent
  extend T::Helpers

  sealed!
end

class Child1; include Parent; end

class Child2; include Parent; end

def example
  c1 = Child1.new
  c1.
  #  ^ completion: class, ...
  c2 = Child2.new
  c2.
  #  ^ completion: class, ...
end # error: unexpected token "end"
