# frozen_string_literal: true
# typed: false
require_relative '../test_helper'

module Opus::Types::Test
  class LambdaTest < Critic::Unit::UnitTest
    it "takes positional arguments" do
      lambda = T.lambda do |x: Integer, y: Integer|
        x + y
      end

      assert_equal(5, lambda.call(2, 3))
    end

    it "raises an error if arity does not match" do
      lambda = T.lambda {|x: Integer| x + 1}

      assert_raises(ArgumentError) do
        lambda.call
      end

      assert_raises(ArgumentError) do
        lambda.call(2, 3)
      end
    end

    it "raises an error if the parameters are not kwarg shaped" do
      lambda = T.lambda {|x| x + 1}

      assert_raises(TypeError) do
        lambda.call(5)
      end
    end
  end
end
