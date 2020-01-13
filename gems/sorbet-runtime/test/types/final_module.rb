# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::FinalModuleTest < Critic::Unit::UnitTest
  before do
    T::Configuration.enable_final_checks_on_hooks
  end

  after do
    T::Private::DeclState.current.reset!
    T::Configuration.reset_final_checks_on_hooks
  end

  it "allows declaring a class as final" do
    Class.new do
      extend T::Helpers
      final!
    end
  end

  it "allows declaring a module as final" do
    Module.new do
      extend T::Helpers
      final!
    end
  end

  it "forbids inheriting a final class" do
    c = Class.new do
      extend T::Helpers
      final!
    end
    err = assert_raises(RuntimeError) do
      Class.new(c)
    end
    assert_match(/^#<Class:0x[0-9a-f]+> was declared as final and cannot be inherited$/, err.message)
  end

  it "forbids including a final module" do
    m = Module.new do
      extend T::Helpers
      final!
    end
    err = assert_raises(RuntimeError) do
      Module.new do
        include m
      end
    end
    assert_match(/^#<Module:0x[0-9a-f]+> was declared as final and cannot be included$/, err.message)
  end

  it "forbids extending a final module" do
    m = Module.new do
      extend T::Helpers
      final!
    end
    err = assert_raises(RuntimeError) do
      Module.new do
        extend m
      end
    end
    assert_match(/^#<Module:0x[0-9a-f]+> was declared as final and cannot be extended$/, err.message)
  end

  it "allows declaring a module as final and its instance method as final" do
    Module.new do
      extend T::Helpers
      final!
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
  end

  it "allows declaring a module as final and its class method as final" do
    Module.new do
      extend T::Helpers
      final!
      extend T::Sig
      sig(:final) {void}
      def self.foo; end
    end
  end

  it "forbids declaring a module as final but not its instance method as final" do
    err = assert_raises(RuntimeError) do
      Module.new do
        extend T::Helpers
        final!
        extend T::Sig
        def foo; end
      end
    end
    assert_match(/^#<Module:0x[0-9a-f]+> was declared as final but its method `foo` was not declared as final$/, err.message)
  end

  it "forbids declaring a module as final but not its class method as final" do
    err = assert_raises(RuntimeError) do
      Module.new do
        extend T::Helpers
        final!
        extend T::Sig
        def self.foo; end
      end
    end
    assert_match(/^#<Class:#<Module:0x[0-9a-f]+>> was declared as final but its method `foo` was not declared as final$/, err.message)
  end

  it "forbids declaring a class as final and then abstract" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        final!
        abstract!
      end
    end
    assert_match(/^#<Class:0x[0-9a-f]+> was already declared as final and cannot be declared as abstract$/, err.message)
  end

  it "forbids declaring a class as abstract and then final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        abstract!
        final!
      end
    end
    assert_match(/^#<Class:0x[0-9a-f]+> was already declared as abstract and cannot be declared as final$/, err.message)
  end

  it "forbids re-declaring a class as final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        final!
        final!
      end
    end
    assert_match(/^#<Class:0x[0-9a-f]+> was already declared as final and cannot be re-declared as final$/, err.message)
  end

  it "forbids declaring a non-module, non-method as final" do
    err = assert_raises(RuntimeError) do
      Array.new(1) do
        extend T::Helpers
        final!
      end
    end
    assert_match(/^#<Opus::Types::Test::FinalModuleTest:0x[0-9a-f]+> is not a class or module and cannot be declared as final with `final!`$/, err.message)
  end

  it "allows a non-final class in a final class" do
    inner = nil
    outer = Class.new do
      extend T::Helpers
      final!
      inner = Class.new
    end
    Class.new(inner)
    err = assert_raises(RuntimeError) { Class.new(outer) }
    assert_match(/^#<Class:0x[0-9a-f]+> was declared as final and cannot be inherited$/, err.message)
  end

  it "allows a non-final class in a final module" do
    inner = nil
    outer = Module.new do
      extend T::Helpers
      final!
      inner = Class.new
    end
    Class.new(inner)
    err = assert_raises(RuntimeError) { Class.new.include(outer) }
    assert_match(/^#<Module:0x[0-9a-f]+> was declared as final and cannot be included$/, err.message)
  end

  it "allows a non-final module in a final class" do
    inner = nil
    outer = Class.new do
      extend T::Helpers
      final!
      inner = Module.new
    end
    Class.new.include(inner)
    err = assert_raises(RuntimeError) { Class.new(outer) }
    assert_match(/^#<Class:0x[0-9a-f]+> was declared as final and cannot be inherited$/, err.message)
  end

  it "allows a non-final module in a final module" do
    inner = nil
    outer = Module.new do
      extend T::Helpers
      final!
      inner = Module.new
    end
    Class.new.include(inner)
    err = assert_raises(RuntimeError) { Class.new.include(outer) }
    assert_match(/^#<Module:0x[0-9a-f]+> was declared as final and cannot be included$/, err.message)
  end

  it "allows inheriting a class with hooks" do
    parent = Class.new do
      extend T::Sig
      sig {void}
      def foo; end
    end
    Class.new(parent) do
      extend T::Helpers
      final!
    end
  end
end
