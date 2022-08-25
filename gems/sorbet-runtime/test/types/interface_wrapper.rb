# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::InterfaceWrapperTest < Critic::Unit::UnitTest
  module BaseMixin
    def base; end
  end

  module Mixin
    include BaseMixin

    def foo(bar)
      bar + yield
    end

    private def priv
      42
    end

    def kwarg(bar:)
      bar + 1
    end
  end

  module Mixin2
    def mixin2
      nil
    end
  end

  class Implementation
    include Mixin
    include Mixin2

    def bar
      43
    end
  end

  before do
    @obj = Implementation.new
    @wrapper = T::InterfaceWrapper.wrap_instance(@obj, Mixin)
    @array = T::InterfaceWrapper.wrap_instances([@obj], Mixin)
  end

  describe "method calls" do
    it "proxies methods in the module" do
      assert_equal("ab", @wrapper.foo("a") {"b"})
    end

    it "does not proxy methods not in the module" do
      assert_raises(NoMethodError) do
        @wrapper.bar
      end
    end

    it "keeps private methods private" do
      err = assert_raises(NoMethodError) do
        @wrapper.priv
      end

      assert_includes(err.message, "private method `priv'")
      assert_equal(42, @wrapper.send(:priv))
    end
  end

  describe "dynamic_cast" do
    it "returns nil for an unrelated type" do
      res = T::InterfaceWrapper.dynamic_cast(@wrapper, Array)
      assert_nil(res)
    end

    it "returns the same object when it matches the type and is not a wrapper" do
      foo = Object.new
      res = T::InterfaceWrapper.dynamic_cast(foo, Object)
      assert_same(foo, res)
    end

    it "returns the wrapped object when passing the wrapped object's type" do
      res = T::InterfaceWrapper.dynamic_cast(@wrapper, Implementation)
      assert_same(@obj, res)
    end
  end

  describe "wrapped_dynamic_cast" do
    it "returns the wrapped object when passing the wrapped object's type" do
      res = T::InterfaceWrapper.wrapped_dynamic_cast(@wrapper, Implementation)
      assert_same(@obj, res)
    end

    it "returns a proxy to a different module on the wrapped object" do
      res = T::InterfaceWrapper.wrapped_dynamic_cast(@wrapper, Mixin2)
      assert_kind_of(T::InterfaceWrapper, @wrapper)
      assert_respond_to(res, :mixin2)
      refute_respond_to(res, :foo)
      refute_respond_to(res, :bar)
    end

    it "doesn't create a wrapper to a wrapper when upcasting on a wrapper" do
      res = T::InterfaceWrapper.wrapped_dynamic_cast(@wrapper, BaseMixin)
      refute_kind_of(T::InterfaceWrapper, res.__target_obj_DO_NOT_USE)
    end

    it "returns the same object when asking for an indentical wrapper" do
      res = T::InterfaceWrapper.wrapped_dynamic_cast(@wrapper, Mixin)
      assert_same(res, @wrapper)
    end
  end

  describe "is_a?" do
    it "returns true for the interface module" do
      assert_kind_of(Mixin, @wrapper)
    end

    it "returns true for InterfaceWrapper" do
      assert_kind_of(T::InterfaceWrapper, @wrapper)
    end

    it "returns false for the object class" do
      refute_kind_of(Implementation, @wrapper)
    end
  end

  describe "array wrapper" do
    it "returns object in array" do
      assert_kind_of(Mixin, @array[0])
      res = T::InterfaceWrapper.wrapped_dynamic_cast(@array[0], Implementation)
      assert_same(@obj, res)
    end

    it "raises if any element is not castable" do
      assert_raises(RuntimeError) {T::InterfaceWrapper.wrap_instances([@obj, nil], Mixin)}
      assert_raises(RuntimeError) {T::InterfaceWrapper.wrap_instances([@obj, "foo"], Mixin)}
      assert_raises(RuntimeError) {T::InterfaceWrapper.wrap_instances([@obj, 5], Mixin)}
    end
  end

  describe "argument forwarding" do
    it "does not cause Ruby keyword argument warnings" do
      # this test has a few different failure modes:
      # - on 2.6 and below, we want to make sure we don't call the
      #   `ruby2_keywords` helper method because it doesn't exist yet
      # - on 2.7, we want to make sure we don't get warnings about
      #   bad kwarg usage: to do this, we enable the relevant deprecation
      #   warnings for the duration of the test and assert that the warn
      #   method is never called
      # - on 3 and above, simply calling a wrapped kwarg method is
      #   sufficient to show that the behavior is correct: if we didn't
      #   specify `ruby2_keywords`, then it would fail with a "wrong
      #   number of arguments" error.

      if RUBY_VERSION.start_with?("2.7")
        Warning[:deprecated] = true
        replaced = T::Private::ClassUtils.replace_method(Warning, :warn) do |warning|
          raise "Found kwarg warning: #{warning}"
        end
      end
      @wrapper.kwarg(bar: 5)
    ensure
      if RUBY_VERSION.start_with?("2.7")
        replaced&.restore
        Warning[:deprecated] = false
      end
    end
  end
end
