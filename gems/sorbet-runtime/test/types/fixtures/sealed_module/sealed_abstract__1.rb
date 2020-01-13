module Opus::Types::Test::SealedModuleSandbox
  class AbstractSealedParent
    extend T::Helpers
    abstract!
    sealed!
  end

  class AbstractSealedChildGood < AbstractSealedParent; end

  class SealedAbstractParent
    extend T::Helpers
    sealed!
    abstract!
  end

  class SealedAbstractChildGood < SealedAbstractParent; end
end
