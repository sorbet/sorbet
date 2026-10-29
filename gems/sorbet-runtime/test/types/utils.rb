# frozen_string_literal: true
# typed: ignore
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

      it 'reports the names and types of rest, keyrest, and block params' do
        c = Class.new do
          extend T::Sig
          sig { params(a: Integer, rest: String, kw: Integer, opts: Symbol, blk: T.proc.void).void }
          def all_kinds(a, *rest, kw:, **opts, &blk); end

          sig { params(a: Integer).void }
          def plain(a); end
        end

        sig = T::Utils.signature_for_method(c.instance_method(:all_kinds))
        assert_equal(:rest, sig.rest_name)
        assert_equal('String', sig.rest_type.name)
        assert_equal(:opts, sig.keyrest_name)
        assert_equal('Symbol', sig.keyrest_type.name)
        assert_equal(:blk, sig.block_name)
        refute_nil(sig.block_type)

        plain = T::Utils.signature_for_method(c.instance_method(:plain))
        assert_nil(plain.rest_name)
        assert_nil(plain.rest_type)
        assert_nil(plain.keyrest_name)
        assert_nil(plain.keyrest_type)
        assert_nil(plain.block_name)
        assert_nil(plain.block_type)
      end

      it 'reports the method parameters' do
        c = Class.new do
          extend T::Sig
          sig { params(a: Integer, b: String, blk: T.proc.void).void }
          def fixed(a, b='', &blk); end

          sig { params(a: Integer, rest: String).void }
          def with_rest(a, *rest); end
        end

        assert_equal([%i[req a], %i[opt b], %i[block blk]], T::Utils.signature_for_method(c.instance_method(:fixed)).parameters)
        assert_equal([%i[req a], %i[rest rest]], T::Utils.signature_for_method(c.instance_method(:with_rest)).parameters)
      end

      it 'keeps explicitly passed parameters' do
        sig = T::Private::Methods::Signature.new_untyped(method: Array.instance_method(:push))
        assert_equal([%i[rest arg0]], sig.parameters)
      end

      it 'names the rest param in call errors' do
        c = Class.new do
          extend T::Sig
          sig { params(a: Integer, rest: String).void }
          def with_rest(a, *rest); end
        end

        err = assert_raises(TypeError) { c.new.with_rest(1, 'ok', :bad) }
        assert_match(/Parameter 'rest': Expected type String, got type Symbol/, err.message)
      end

      it 'names an unnamed rest param positionally on untyped signatures' do
        sig = T::Private::Methods::Signature.new_untyped(method: Array.instance_method(:push))
        assert_equal(:arg0, sig.rest_name)
        assert_nil(sig.keyrest_name)
        assert_nil(sig.block_name)
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

    describe 'run_all_sig_blocks' do
      it 'raises if pending sigs' do
        Module.new do
          extend T::Sig
          sig { void }
        end
        exn = assert_raises(RuntimeError) do
          T::Utils.run_all_sig_blocks
        end
        assert_match(/pending `sig` block in/, exn.message)
      end
    end
  end
end
