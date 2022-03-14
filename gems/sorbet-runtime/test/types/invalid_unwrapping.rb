# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class InvalidUnwrappingTest < Critic::Unit::UnitTest
    it "prevents re-declaring the same method without a signature" do
      klass = Class.new do
        extend T::Sig

        def initialize
          @double_definition = 1
        end

        sig {returns(Integer)}
        def double_definition
          5
        end

        attr_reader :double_definition
      end

      unwrap_signature("double_definition")
      assert_equal(1, klass.new.double_definition)
    end

    it "does not prevent re-declaring if both definitions have signatures" do
      klass = Class.new do
        extend T::Sig

        sig {returns(Integer)}
        def good_double_definition
          5
        end

        sig {returns(Integer)}
        def good_double_definition
          1
        end
      end

      unwrap_signature("good_double_definition")
      assert_equal(1, klass.new.good_double_definition)
    end

    it "does not prevent re-declaring if not using signatures" do
      klass = Class.new do
        def good_double_definition
          5
        end

        def good_double_definition
          1
        end
      end

      unwrap_signature("good_double_definition")
      assert_equal(1, klass.new.good_double_definition)
    end

    it "does not prevent re-declaring if only the latest definition has a signature" do
      klass = Class.new do
        extend T::Sig

        def good_double_definition
          5
        end

        sig {returns(Integer)}
        def good_double_definition
          1
        end
      end

      unwrap_signature("good_double_definition")
      assert_equal(1, klass.new.good_double_definition)
    end

    it "prevents unwrapping for abstract methods if they have been overridden" do
      klass = Class.new do
        extend T::Sig
        extend T::Helpers

        def initialize
          @double_definition = 1
        end

        sig {abstract.returns(Integer)}
        def double_definition; end

        attr_reader :double_definition
      end

      unwrap_signature("double_definition")
      assert_equal(1, klass.new.double_definition)
    end

    it "does not prevent re-declaring a method as abstract" do
      mod = Module.new do
        attr_reader :double_definition
      end

      klass = Class.new do
        extend T::Sig
        extend T::Helpers
        include mod

        def initialize
          @double_definition = 1
        end

        sig {abstract.returns(Integer)}
        def double_definition; end
      end

      unwrap_signature("double_definition")
      assert_equal(1, klass.new.double_definition)
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
