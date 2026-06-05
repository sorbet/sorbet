# typed: true
class Module; include T::Sig; end

module Parent
  extend T::Helpers

  sealed!
end

class Child1; include Parent; end

class Child2; include Parent; end

sig { params(parent: Parent).void }
def example(parent)
  parent.cas
  #      ^^^ error: does not exist
  #         ^ apply-completion: [A] item: 0
    
  x = parent.case
  #          ^^^^ error: does not exist
  #              ^ apply-completion: [C] item: 0

  parent.
  #      ^ apply-completion: [D] item: 7
end # error: unexpected token "end"
