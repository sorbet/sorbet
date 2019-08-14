module Opus::Types::Test::SealedModuleSandbox
  class AbstractParent
    extend T::Helpers

    abstract!
    sealed!
  end

  class ChildGood1 < AbstractParent; end
  ChildGood2 = Class.new(AbstractParent)
end
