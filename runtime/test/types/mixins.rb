# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class MixinsTest < Critic::Unit::UnitTest
    it 'mixes in class methods' do
      mixin = Module.new do
        extend T::Helpers
        mixes_in_class_methods(Module.new do
                                 def mixin_class_method; end
                               end)
      end

      klass = Class.new do
        include mixin
      end

      klass.mixin_class_method
    end

    it 'composes with a self.included' do
      mixin = Module.new do
        extend T::Helpers
        mixes_in_class_methods(Module.new do
                                 def mixin_class_method; end
                               end)

        def self.included(other)
          super

          def other.other_class_method; end
        end
      end

      klass = Class.new do
        include mixin
      end

      klass.mixin_class_method
      klass.other_class_method
    end

    it 'cannot be used on a Class' do
      ex = assert_raises(RuntimeError) do
        Class.new do
          extend T::Helpers

          mixes_in_class_methods(Module.new {})
        end
      end

      assert_includes(ex.message,
                      "mixes_in_class_methods cannot be used on a Class")
    end

    it 'cannot be used twice' do
      ex = assert_raises(RuntimeError) do
        Module.new do
          extend T::Helpers

          mixes_in_class_methods(Module.new {})
          mixes_in_class_methods(Module.new {})
        end
      end

      assert_includes(ex.message,
                      "mixes_in_class_methods can only be used once")
    end
  end
end
