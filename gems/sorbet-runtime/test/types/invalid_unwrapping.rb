# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class InvalidUnwrappingTest < Critic::Unit::UnitTest
    it "respects the latest override of a method if it does not have a signature" do
      klass = Class.new do
        extend T::Sig

        def initialize
          @double_definition = "OK"
        end

        sig {returns(String)}
        def double_definition
          "FAIL"
        end

        attr_reader :double_definition
      end

      unwrap_signature("double_definition")
      assert_equal("OK", klass.new.double_definition)
    end

    it "respects the latest override of a method if it has a signature" do
      klass = Class.new do
        extend T::Sig

        sig {returns(String)}
        def good_double_definition
          "FAIL"
        end

        sig {returns(String)}
        def good_double_definition
          "OK"
        end
      end

      unwrap_signature("good_double_definition")
      assert_equal("OK", klass.new.good_double_definition)
    end

    it "respects the latest override of a method if none of them have signatures" do
      klass = Class.new do
        def good_double_definition
          "FAIL"
        end

        def good_double_definition
          "OK"
        end
      end

      unwrap_signature("good_double_definition")
      assert_equal("OK", klass.new.good_double_definition)
    end

    it "respects the latest override of a method if only the last one has a signature" do
      klass = Class.new do
        extend T::Sig

        def good_double_definition
          "FAIL"
        end

        sig {returns(String)}
        def good_double_definition
          "OK"
        end
      end

      unwrap_signature("good_double_definition")
      assert_equal("OK", klass.new.good_double_definition)
    end

    it "respects the latest override of an abstract method if it does not have a signature" do
      klass = Class.new do
        extend T::Sig
        extend T::Helpers

        def initialize
          @double_definition = "OK"
        end

        sig {abstract.returns(String)}
        def double_definition; end

        attr_reader :double_definition
      end

      unwrap_signature("double_definition")
      assert_equal("OK", klass.new.double_definition)
    end

    it "respects the latest override of a method if the latest one is abstract" do
      mod = Module.new do
        attr_reader :double_definition
      end

      klass = Class.new do
        extend T::Sig
        extend T::Helpers
        include mod

        def initialize
          @double_definition = "OK"
        end

        sig {abstract.returns(String)}
        def double_definition; end
      end

      unwrap_signature("double_definition")
      assert_equal("OK", klass.new.double_definition)
    end

    private

    def unwrap_signature(method_name)
      # Run the signature block only for `double_definition` to simulate what would happen when invoking
      # run_all_sig_blocks
      wrappers = T::Private::Methods.instance_variable_get(:@sig_wrappers)
      key = wrappers.keys.find {|k| k.end_with?(method_name)}
      T::Private::Methods.maybe_run_sig_block_for_key(key)
    end
  end
end
