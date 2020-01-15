module Opus::Types::Test::SealedModuleSandbox
  T::Configuration.sealed_violation_whitelist = [/whitelist_violation__1\.rb/]

  class SealedParent
    extend T::Helpers
    abstract!
    sealed!
  end

  class SealedChildGood < SealedParent; end
end
