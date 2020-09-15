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

    it 'mixes in class methods with multiple arguments' do
      mixin = Module.new do
        extend T::Helpers
        mixes_in_class_methods(Module.new do
                                 def mixin_class_method_1; end
                               end,
                               Module.new do
                                 def mixin_class_method_2; end
                               end)
        mixes_in_class_methods(Module.new do
                                 def mixin_class_method_3; end
                               end)
      end

      klass = Class.new do
        include mixin
      end

      klass.mixin_class_method_1
      klass.mixin_class_method_2
      klass.mixin_class_method_3
    end

    it 'correctly modifies expected ancestor chain ordering when called with multiple arguments' do
      mixin = Module.new do
        extend T::Helpers
        mixes_in_class_methods(Module.new do
                                 def mixin_class_method; 1; end
                               end,
                               Module.new do
                                 def mixin_class_method; 2; end
                               end)
      end

      klass = Class.new { include mixin }
      result = klass.mixin_class_method
      assert_equal(2, result)
    end

    it 'correctly modifies ancestor chain ordering when called multiple times with multiple arguments' do
      mixin = Module.new do
        extend T::Helpers
        mixes_in_class_methods(Module.new do
                                 def mixin_class_method; 1; end
                               end,
                               Module.new do
                                 def mixin_class_method; 2; end
                               end)
        mixes_in_class_methods(Module.new do
                                def mixin_class_method; 3; end
                               end,
                               Module.new do
                                def mixin_class_method; 4; end
                               end)
      end

      klass = Class.new { include mixin }
      result = klass.mixin_class_method
      assert_equal(4, result)
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
  end
end
