# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::FinalModuleTest < Critic::Unit::UnitTest
  before do
    T::Configuration.enable_final_checks_for_include_extend
  end

  after do
    T::Private::DeclState.current.reset!
    T::Configuration.reset_final_checks_for_include_extend
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
    assert_includes(err.message, "was declared as final and cannot be inherited")
  end

  it "allows declaring a module as final and its method as final" do
    Module.new do
      extend T::Helpers
      final!
      extend T::Sig
      sig(:final) {void}
      def self.foo; end
    end
  end

  it "forbids declaring a module as final but not its method as final" do
    err = assert_raises(RuntimeError) do
      Module.new do
        extend T::Helpers
        final!
        extend T::Sig
        def self.foo; end
      end
    end
    assert_includes(err.message, "was declared as final but its method")
    assert_includes(err.message, "was not declared as final")
  end

  it "forbids declaring a class as final and then abstract" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        final!
        abstract!
      end
    end
    assert_includes(err.message, "was already declared as final and cannot be declared as abstract")
  end

  it "forbids declaring a class as abstract and then final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        abstract!
        final!
      end
    end
    assert_includes(err.message, "was already declared as abstract and cannot be declared as final")
  end

  it "forbids re-declaring a class as final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        final!
        final!
      end
    end
    assert_includes(err.message, "was already declared as final and cannot be re-declared as final")
  end

  it "forbids declaring a non-module, non-method as final" do
    err = assert_raises(RuntimeError) do
      Array.new(1) do
        extend T::Helpers
        final!
      end
    end
    assert_includes(err.message, "is not a module or method and cannot be declared as final")
  end
end
