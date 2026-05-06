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

    describe 'T::Utils.first_non_sorbet_caller' do
      extend T::Sig

      # rubocop:disable Style/ParallelAssignment

      it 'returns the caller frame when called from a sig-wrapped method' do
        expected_line, loc = [__LINE__, sigged_helper]

        assert_equal(__FILE__, loc.path)
        assert_equal(expected_line, loc.lineno)
      end

      it 'skips multiple sorbet-runtime frames inserted by nested sig wrappers' do
        expected_line, loc = sigged_outer

        assert_equal(__FILE__, loc.path)
        assert_equal(expected_line, loc.lineno)
      end

      it 'returns the caller when called from a non-sig method' do
        expected_line, loc = [__LINE__, plain_helper]

        assert_equal(__FILE__, loc.path)
        assert_equal(expected_line, loc.lineno)
      end

      # rubocop:enable Style/ParallelAssignment

      private

      sig { returns(T.nilable(Thread::Backtrace::Location)) }
      def sigged_helper
        T::Utils.first_non_sorbet_caller
      end

      sig { returns([Integer, T.nilable(Thread::Backtrace::Location)]) }
      def sigged_outer
        expected_line = __LINE__ + 1
        loc = sigged_helper
        [expected_line, loc]
      end

      def plain_helper
        T::Utils.first_non_sorbet_caller
      end
    end
  end
end
