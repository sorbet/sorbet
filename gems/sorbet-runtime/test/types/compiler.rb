# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class CompilerTest < Critic::Unit::UnitTest
    describe 'T::Private::Compiler.running_compiled?' do
      it 'returns false always, because this is the interpreter' do
        refute(T::Private::Compiler.running_compiled?)
      end
    end

    describe 'T::Private::Compiler.compiler_version' do
      it 'returns nil always, because this is the interpreter' do
        assert_nil(T::Private::Compiler.compiler_version)
      end
    end
  end
end
