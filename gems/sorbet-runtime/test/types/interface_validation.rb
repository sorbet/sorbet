# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::InterfacesTest < Critic::Unit::UnitTest
  it "succeeds when all methods are declared as abstract" do
    base = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.returns(Object)}
      def foo; end
    end

    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!
      include base

      sig {abstract.returns(Object)}
      def bar; end
    end

    T::Private::Abstract::Validate.validate_abstract_module(mod)
  end

  it "raises an error if an interface has a method without a type signature" do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.returns(Object)}
      def good; end

      def bad; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end
    assert_includes(err.message, "not declared with `abstract`")
    assert_includes(err.message, "`bad`")
  end

  it "raises an error if an interface has a method with a non-abstract type signature" do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.returns(Object)}
      def good; end

      sig {returns(Object)}
      def bad; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end
    assert_includes(err.message, "not declared with `abstract`")
    assert_includes(err.message, "`bad`")
  end

  it "raises an error if an interface inherits a method without a type signature" do
    mixin = Module.new do
      def bad; end
    end
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!
      include mixin

      sig {abstract.returns(Object)}
      def good; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end
    assert_includes(err.message, "not declared with `abstract`")
    assert_includes(err.message, "`bad`")
  end

  it "raises an error if an interface has a private method" do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.returns(Object)}
      def good; end

      sig {abstract.returns(Object)}
      private def bad; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end
    assert_includes(err.message, "All methods on an interface must be public.")
    assert_includes(err.message, "`bad`")
  end

  it "raises an error if an interface has a protected method" do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.returns(Object)}
      protected def bad; end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end
    assert_includes(err.message, "All methods on an interface must be public.")
    assert_includes(err.message, "`bad`")
  end

  it "allows void methods in interfaces" do
    base = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.void}
      def foo; end
    end

    Module.new do
      extend T::Sig
      extend T::Helpers
      include base

      sig {override.void}
      def foo; end
    end

    T::Private::Abstract::Validate.validate_abstract_module(base)
  end

  it "does not raise an error if a void method does not have a void implementation" do
    base = Module.new do
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.void}
      def foo; end
    end

    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      include base

      sig {override.returns(Integer)}
      def foo
        1
      end
    end

    klass = Class.new do
      include mod
    end

    klass.new.foo
  end

end
