# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class MethodPatchesTest < Critic::Unit::UnitTest
    module MethodRefinement
      refine Method do
        T::CompatibilityPatches::MethodExtensions.instance_methods(false).each do |method|
          define_method(
            method,
            T::CompatibilityPatches::MethodExtensions.instance_method(method),
          )
        end
      end
    end

    using MethodRefinement

    before do
      @mod = Module.new do
        extend T::Sig
        # Make it public for testing only
        public_class_method :sig
      end
    end

    it "return the same result for simple methods with and without signature" do
      def @mod.no_sig(bar)
        :foo
      end

      @mod.sig {params(bar: T.untyped).returns(T.untyped)}
      def @mod.with_sig(bar)
        :foo
      end

      method_no_sig = @mod.method(:no_sig)
      method_with_sig = @mod.method(:with_sig)

      assert_equal(method_no_sig.arity, method_with_sig.arity)
      assert_equal(method_no_sig.parameters, method_with_sig.parameters)
      # file names from source_location should match
      assert_equal(method_no_sig.source_location.first, method_with_sig.source_location.first)
      # line numbers from source_location should differ by 5 lines
      assert_equal(method_no_sig.source_location.last + 5, method_with_sig.source_location.last)
    end

    it "return the same result for rest arg methods with and without signature" do
      def @mod.no_sig(*bar)
        :foo
      end

      @mod.sig {params(bar: T.untyped).returns(T.untyped)}
      def @mod.with_sig(*bar)
        :foo
      end

      method_no_sig = @mod.method(:no_sig)
      method_with_sig = @mod.method(:with_sig)

      assert_equal(method_no_sig.arity, method_with_sig.arity)
      assert_equal(method_no_sig.parameters, method_with_sig.parameters)
      # file names from source_location should match
      assert_equal(method_no_sig.source_location.first, method_with_sig.source_location.first)
      # line numbers from source_location should differ by 5 lines
      assert_equal(method_no_sig.source_location.last + 5, method_with_sig.source_location.last)
    end

    it "return the same result for complex methods with and without signature" do
      def @mod.no_sig(a, b=nil, *c, d:, e: nil, **f, &blk)
        :foo
      end

      @mod.sig {params(a: T.untyped, b: T.untyped, c: T.untyped, d: T.untyped, e: T.untyped, f: T.untyped, blk: T.untyped).void}
      def @mod.with_sig(a, b=nil, *c, d:, e: nil, **f, &blk)
        :foo
      end

      method_no_sig = @mod.method(:no_sig)
      method_with_sig = @mod.method(:with_sig)

      assert_equal(method_no_sig.arity, method_with_sig.arity)
      assert_equal(method_no_sig.parameters, method_with_sig.parameters)
      # file names from source_location should match
      assert_equal(method_no_sig.source_location.first, method_with_sig.source_location.first)
      # line numbers from source_location should differ by 5 lines
      assert_equal(method_no_sig.source_location.last + 5, method_with_sig.source_location.last)
    end
  end
end
