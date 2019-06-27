# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::StructValidationTest < Critic::Unit::UnitTest
  it "forbids subclassing a Struct" do
    c = Class.new(T::Struct)
    err = assert_raises(RuntimeError) do
      Class.new(c)
    end
    assert_includes(err.message, "is a subclass of T::Struct and cannot be subclassed")
  end

  it "allows subclassing an InexactStruct" do
    c = Class.new(T::InexactStruct)
    Class.new(c)
  end

  it "forbids subclassing an InexactStruct that includes FinalStruct" do
    c = Class.new(T::InexactStruct) do
      include T::FinalStruct
    end
    err = assert_raises(RuntimeError) do
      Class.new(c)
    end
    assert_includes(err.message, "includes T::FinalStruct and cannot be subclassed")
  end

  it "forbids subclassing a deep subclass of InexactStruct that includes FinalStruct" do
    c1 = Class.new(T::InexactStruct)
    c2 = Class.new(c1) do
      include T::FinalStruct
    end
    err = assert_raises(RuntimeError) do
      Class.new(c2)
    end
    assert_includes(err.message, "includes T::FinalStruct and cannot be subclassed")
  end
end
