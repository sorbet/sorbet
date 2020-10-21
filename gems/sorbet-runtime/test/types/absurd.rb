# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class AbsurdTest < Critic::Unit::UnitTest
    it 'raises when called' do
      ex = assert_raises(TypeError) do
        T.absurd(nil)
      end
      assert_includes(ex.message, 'Control flow reached T.absurd. Got value:')
    end

    it 'calls the T::Configuration handler' do
      begin
        T::Configuration.inline_type_error_handler = lambda do |_ex|
          raise "Called custom handler"
        end
        ex = assert_raises(RuntimeError) do
          T.absurd(nil)
        end
        assert_equal(ex.message, 'Called custom handler')
      ensure
        T::Configuration.inline_type_error_handler = nil
      end
    end
  end
end
