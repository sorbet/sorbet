# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class UtilsTest < Critic::Unit::UnitTest
    describe 'T::Utils.unwrap_nilable' do
      it 'unwraps when multiple elements' do
        type = T.any(String, NilClass, Float)
        unwrapped = T.must(T::Utils.unwrap_nilable(type))

        assert(T.any(String, Float).subtype_of?(unwrapped))
        assert(unwrapped.subtype_of?(T.any(String, Float)))
      end

      it 'unwraps with a simple pair' do
        type = T.any(String, Float)
        assert_nil(T::Utils.unwrap_nilable(type))
      end
    end

    describe 'T::Utils.signature_for_method' do
      it 'returns nil on methods without sigs' do
        c = Class.new do
          def no_sig; end
        end
        assert_nil(T::Utils.signature_for_method(c.instance_method(:no_sig)))
      end

      it 'returns nil on secretly-defined methods with no sigs' do
        c = Class.new do
          T::Private::Methods._with_declared_signature(self, nil) do
            def no_sig; end
          end
        end
        assert_nil(T::Utils.signature_for_method(c.instance_method(:no_sig)))
      end

      it 'returns things on methods with sigs' do
        c = Class.new do
          extend T::Sig
          sig { returns(Integer) }
          def sigfun
            85
          end
        end
        sfm = T::Utils.signature_for_method(c.instance_method(:sigfun))
        refute_nil(sfm)
        assert_equal(:sigfun, sfm.method.name)
        assert_equal(:sigfun, sfm.method_name)
        assert_equal('Integer', sfm.return_type.name)
      end

      it 'returns things on secretly-defined methods with sigs' do
        c = Class.new do
          built_sig = T::Private::Methods._declare_sig(self) do
            returns(Integer)
          end

          T::Private::Methods._with_declared_signature(self, built_sig) do
            def sigfun
              85
            end
          end
        end
        sfm = T::Utils.signature_for_method(c.instance_method(:sigfun))
        refute_nil(sfm)
        assert_equal(:sigfun, sfm.method.name)
        assert_equal(:sigfun, sfm.method_name)
        assert_equal('Integer', sfm.return_type.name)
      end
    end

    describe 'force_type_init' do
      it 'works' do
        Class.new do
          extend T::Sig

          sig { params(x: Integer).returns(String) }
          def foo(x); x.to_s; end

          T::Private::Methods.send(
            :run_sig_block_for_key,
            T::Private::Methods.send(:method_to_key, instance_method(:foo)),
            force_type_init: true
          )
        end
      end
    end

    describe 'T::Utils.unload_const' do
      it 'releases const references from T::Private::Methods' do
        foo_module = Module.new do
          extend T::Sig

          sig { void }
          def self.method_1; end

          sig { void }
          private_class_method def self.method_2; end

          sig { void }
          def method_3; end

          protected

          sig { void }
          def method_4; end

          private

          sig { void }
          def method_5; end
        end
        
        bar_class = Class.new do
          extend T::Sig

          class << self
            extend T::Sig

            sig { void }
            def method_6; end
          end

          sig { void }
          def self.method_7; end

          sig { void }
          private_class_method def self.method_8; end

          sig { void }
          def method_9; end

          private

          sig { returns(T.untyped) }
          def method_10; end

          protected

          sig { void }
          def method_11; end
        end
        
        self.class.const_set(:Foo, foo_module)
        foo_module.const_set(:Bar, bar_class)

        mod = self.class::Foo
        inner_class = self.class::Foo::Bar

        mod_id = mod.object_id
        mod_singleton_class_id = mod.singleton_class.object_id
        inner_class_id = inner_class.object_id
        inner_class_singleton_class_id = inner_class.singleton_class.object_id

        method_ids = [
          "#{mod_singleton_class_id}#method_1",
          "#{mod_singleton_class_id}#method_2",
          "#{mod_id}#method_3",
          "#{mod_id}#method_4",
          "#{mod_id}#method_5",
          "#{inner_class_singleton_class_id}#method_6",
          "#{inner_class_singleton_class_id}#method_7",
          "#{inner_class_singleton_class_id}#method_8",
          "#{inner_class_id}#method_9",
          "#{inner_class_id}#method_10",
          "#{inner_class_id}#method_11",
        ]
        inner_singleton_class = inner_class.singleton_class

        installed_hooks_mods = T::Private::Methods.instance_variable_get(:@installed_hooks)[mod]
        installed_hooks_inner_classes = T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_class]
        installed_hooks_inner_singleton_classes = T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_singleton_class]

        assert(!installed_hooks_mods.nil?)
        assert(!installed_hooks_inner_classes.nil?)
        assert(!installed_hooks_inner_singleton_classes.nil?)
        method_ids.each do |method_id|
          sig_wrappers = T::Private::Methods.instance_variable_get(:@sig_wrappers)[method_id]
          assert(!sig_wrappers.nil?, "Expected to find sig wrapper for #{method_id}")
        end

        T::Utils.unload_const(self.class::Foo)

        installed_hooks_mods = T::Private::Methods.instance_variable_get(:@installed_hooks)[mod]
        installed_hooks_inner_classes = T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_class]
        installed_hooks_inner_singleton_classes = T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_singleton_class]

        assert_nil(installed_hooks_mods)
        assert_nil(installed_hooks_inner_classes)
        assert_nil(installed_hooks_inner_singleton_classes)
        method_ids.each do |method_id|
          sig_wrappers = T::Private::Methods.instance_variable_get(:@sig_wrappers)[method_id]
          assert_nil(sig_wrappers)
        end
        
        # Clean up the constant to avoid polluting the test environment
        self.class.send(:remove_const, :Foo) if self.class.const_defined?(:Foo)
      end
    end
  end
end
