# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class ValidateEnumsTest < Critic::Unit::UnitTest

    it "succeeds when implementing Enumerable::each" do
      klass = Class.new(BasicObject) do
        include Enumerable
        extend T::Generic
        extend T::Sig

        sig {
          implementation
            .params(blk: T.proc.params(arg0: Integer).returns(Integer))
            .returns(T::Enumerable[Integer])
        }
        def each(&blk)
          [1, 2, 3].each do |x|
            yield x
          end
        end
      end
      klass.new.each { |x| x + 1 }
    end

    it "raises when implementing a bad interface for Enumerable::each" do
      klass = Class.new(BasicObject) do
        include Enumerable
        extend T::Generic
        extend T::Sig

        sig {
          implementation
            .params(blk: T.proc.params(arg0: Integer).returns(Integer))
            .returns(T::Enumerable[Integer])
        }
        def each(&blk); end # does not return `T::Enumerable[Integer]`, will raise
      end
      err = assert_raises(TypeError) do
        klass.new.each { |x| x + 1 }
      end
      assert_includes(err.message, "Return value: Expected type T::Enumerable[Integer], got type NilClass")
    end

    it "raises when implementing each while not including Enumerable" do
      klass = Class.new(BasicObject) do
        extend T::Generic
        extend T::Sig

        sig {
          implementation
            .params(blk: T.proc.params(arg0: Integer).returns(Integer))
            .returns(T::Enumerable[Integer])
        }
        def each(&blk); end # does not return `T::Enumerable[Integer]`, will raise
      end
      err = assert_raises(RuntimeError) do
        klass.new.each { |x| x + 1 }
      end
      assert_includes(err.message, "You marked `each` as .implementation, but it doesn't match up with a corresponding abstract method.")
    end
  end
end
