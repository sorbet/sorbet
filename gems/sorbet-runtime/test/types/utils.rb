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
          sig {returns(Integer)}
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

    class ForHasSigBlockForMethod
      extend T::Sig

      sig {void}
      def test1; end

      sig {void}
      def test2; end

      T::Sig::WithoutRuntime.sig {void}
      def test3; end
    end

    describe 'T::Utils.has_sig_block_for_method' do
      it 'returns true if the method has a sig block pending' do
        assert(T::Utils.has_sig_block_for_method(ForHasSigBlockForMethod.instance_method(:test1)))
      end

      it 'returns false if the method has already had its sig block forced' do
        assert(T::Utils.has_sig_block_for_method(ForHasSigBlockForMethod.instance_method(:test2)))

        # Another time, to assert that has_sig_block_for_method itself doesn't force it.
        assert(T::Utils.has_sig_block_for_method(ForHasSigBlockForMethod.instance_method(:test2)))

        # Force the sig block
        ForHasSigBlockForMethod.new.test2

        refute(T::Utils.has_sig_block_for_method(ForHasSigBlockForMethod.instance_method(:test2)))
      end

      it 'returns false if the method has no runtime sig wrapper' do
        refute(T::Utils.has_sig_block_for_method(ForHasSigBlockForMethod.instance_method(:test3)))
      end
    end
  end
end
