# typed: true
module Parent
  extend T::Helpers

  sealed!
  
  def case(_) 
  end
end

class Child1; include Parent; end

class Child2; include Parent; end

extend T::Sig

sig { params(parent: Parent).void }
def example(parent)
  parent.cas
  #      ^^^ error: does not exist
  #         ^ apply-completion: [A] item: 0
end
