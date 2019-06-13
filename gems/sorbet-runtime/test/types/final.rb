# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::FinalValidationTest < Critic::Unit::UnitTest
  it "allows declaring a class as final" do
    c = Class.new do
      extend T::Helpers
      __UNSTABLE_final!
      def foo; 3; end
    end
    assert_equal(c.new.foo, 3)
  end

  it "forbids declaring a module as final" do
    err = assert_raises(RuntimeError) do
      Module.new do
        extend T::Helpers
        __UNSTABLE_final!
      end
    end
    assert_includes(err.message, "is not a class (it is a Module), but was declared final")
  end

  it "forbids re-declaring a class as final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        __UNSTABLE_final!
        __UNSTABLE_final!
      end
    end
    assert_includes(err.message, "was already declared as final")
  end

  it "forbids subclassing a final class" do
    c = Class.new do
      extend T::Helpers
      __UNSTABLE_final!
    end
    err = assert_raises(RuntimeError) do
      Class.new(c)
    end
    assert_includes(err.message, "was declared final and cannot be subclassed")
  end

  it "forbids declaring a class as abstract and then final" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        abstract!
        __UNSTABLE_final!
      end
    end
    assert_includes(err.message, "was already declared as abstract and cannot also be declared as final")
  end

  it "forbids declaring a class as final and then abstract" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Helpers
        __UNSTABLE_final!
        abstract!
      end
    end
    assert_includes(err.message, "was already declared as final and cannot also be declared as abstract")
  end
end
