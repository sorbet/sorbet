# frozen_string_literal: true
require_relative '../test_helper'
require_relative '../types/fixtures/reloading_constants'

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

    describe 'T::Utils.unload_const' do
      it 'releases const references from T::Private::Methods' do
        # Read from fixture
        # gems/sorbet-runtime/test/types/fixtures/reloading_constants.rb

        mod = Foo
        inner_class = Foo::Bar

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

        assert(!T::Private::Methods.instance_variable_get(:@installed_hooks)[mod].nil?)
        assert(!T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_class].nil?)
        assert(!T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_singleton_class].nil?)
        method_ids.each do |method_id|
          assert(!T::Private::Methods.instance_variable_get(:@sig_wrappers)[method_id].nil?, "Expected to find sig wrapper for #{method_id}")
        end

        T::Utils.unload_const(Foo)

        assert_nil(T::Private::Methods.instance_variable_get(:@installed_hooks)[mod])
        assert_nil(T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_class])
        assert_nil(T::Private::Methods.instance_variable_get(:@installed_hooks)[inner_singleton_class])
        method_ids.each do |method_id|
          assert_nil(T::Private::Methods.instance_variable_get(:@sig_wrappers)[method_id])
        end
      end
    end
  end
end
