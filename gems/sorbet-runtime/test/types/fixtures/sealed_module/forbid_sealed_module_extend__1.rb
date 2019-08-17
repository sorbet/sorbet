module Opus::Types::Test::SealedModuleSandbox
  module Parent
    extend T::Helpers

    sealed!
  end

  class ChildGood1
    extend Parent
  end

  ChildGood2 = Class.new do
    extend Parent
  end
end
