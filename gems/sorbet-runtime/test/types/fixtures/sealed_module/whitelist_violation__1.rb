module Opus::Types::Test::SealedModuleSandbox
  class SealedParent
    extend T::Helpers
    abstract!
    sealed!
  end

  class SealedChildGood < SealedParent; end
end
