# typed: true

module Test; module Critic; end; end

module Test::Critic
  module Utils
    class UnitTestTest
      include Critic::Core

      Critic::Utils::UnitTest.foo
    end
  end
end
