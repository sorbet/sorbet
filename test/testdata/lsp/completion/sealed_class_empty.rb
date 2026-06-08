# typed: true
extend T::Sig

module Parent
  extend T::Helpers

  sealed!
end

sig { params(parent: Parent).void }
def example(parent)
  parent.cas
  #      ^^^ error: does not exist
  #         ^ apply-completion: [A] item: 0
end
