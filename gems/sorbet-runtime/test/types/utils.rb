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

      it 'names an unnamed rest param positionally on untyped signatures' do
        sig = T::Private::Methods::Signature.new_untyped(method: Array.instance_method(:push))
        assert_equal(:arg0, sig.rest_name)
        assert_nil(sig.keyrest_name)
        assert_nil(sig.block_name)
      end

      it 'reports the names of anonymous rest, keyrest, and block params' do
        c = Class.new do
          extend T::Sig
          sig { params(a: Integer, "*": String, "**": Integer, "&": T.proc.void).void }
          # A string, because RuboCop parses this file as Ruby 3.0, which has no anonymous `&`.
          class_eval('def anonymous(a, *, **, &); end', __FILE__, __LINE__)
        end

        sig = T::Utils.signature_for_method(c.instance_method(:anonymous))
        assert_equal(:*, sig.rest_name)
        assert_equal(:**, sig.keyrest_name)
        assert_equal(:&, sig.block_name)

        names = []
        sig.each_args_value_type([0, 'x', 'y', {k: 1}]) { |name, _, _| names << name }
        assert_equal(%i[a * * k], names)

        err = assert_raises(TypeError) { c.new.anonymous(0, 1) {} }
        assert_match(/Parameter '\*': Expected type String, got type Integer/, err.message)
      end

      it 'names rest args when required params follow the rest param' do
        c = Class.new do
          extend T::Sig
          sig { params(rest: String, a: Integer).void }
          def post_required(*rest, a); end
        end

        sig = T::Utils.signature_for_method(c.instance_method(:post_required))
        assert_equal(:rest, sig.rest_name)

        names = []
        sig.each_args_value_type([0, 'x', 'y']) { |name, _, _| names << name }
        assert_equal(%i[a rest rest], names)
      end

      it 'fits in a 160-byte GC slot' do
        require 'objspace'
        c = Class.new do
          extend T::Sig
          sig { params(a: Integer, rest: String, kw: Integer, opts: Symbol, blk: T.proc.void).void.checked(:always) }
          def all_kinds(a, *rest, kw:, **opts, &blk); end
        end

        sig = T::Utils.signature_for_method(c.instance_method(:all_kinds))
        assert_operator(ObjectSpace.memsize_of(sig), :<=, 160,
                        "T::Private::Methods::Signature grew past 160 bytes; see the comment on its class definition")
      end

      it 'round-trips every combination of packed flags' do
        method = Class.new { def no_params; end }.instance_method(:no_params)
        build = lambda do |mode:, check_level:, override_allow_incompatible: false, defined_raw: false|
          T::Private::Methods::Signature.new(
            method: method, method_name: :no_params, raw_arg_types: {}, raw_return_type: NilClass, bind: nil,
            mode: mode, check_level: check_level, on_failure: nil,
            override_allow_incompatible: override_allow_incompatible, defined_raw: defined_raw,
          )
        end

        T::Private::Methods::Modes::MODES.each do |mode|
          T::Private::RuntimeLevels::LEVELS.each do |check_level|
            [false, true, :visibility].each do |allow_incompatible|
              [false, true].each do |defined_raw|
                sig = build.call(mode: mode, check_level: check_level,
                                 override_allow_incompatible: allow_incompatible, defined_raw: defined_raw)
                assert_equal(
                  [mode, check_level, allow_incompatible, defined_raw],
                  [sig.mode, sig.check_level, sig.override_allow_incompatible, sig.defined_raw],
                )
              end
            end
          end
        end

        assert_equal(T::Private::RuntimeLevels.default_checked_level,
                     build.call(mode: 'standard', check_level: nil).check_level)
        assert_raises(ArgumentError) { build.call(mode: 'bogus', check_level: :always) }
        assert_raises(ArgumentError) { build.call(mode: 'standard', check_level: :bogus) }
        assert_raises(ArgumentError) do
          build.call(mode: 'standard', check_level: :always, override_allow_incompatible: :bogus)
        end
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
