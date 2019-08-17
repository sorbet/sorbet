module Opus::Types::Test::SealedModuleSandbox
  module Parent
    extend T::Helpers

    sealed!
  end

  class ChildGood1
    include Parent
  end

  ChildGood2 = Class.new do
    include Parent
  end
end
