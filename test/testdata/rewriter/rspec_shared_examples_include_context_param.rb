# typed: true
# enable-experimental-requires-ancestor: true

module RSpec
  module Core
    class ExampleGroup
      def expect(arg)
      end

      def eq(arg)
      end
    end
  end
end

class MyClass < RSpec::Core::ExampleGroup
  describe("shared context with params") do
    shared_examples "parameterized shared context" do |my_param|
      let(:param_value) { my_param }

      it "uses the param" do
        T.reveal_type(my_param) # error: `T.untyped`
      end
    end

    shared_context "another parameterized context" do |context_param|
      let(:context_value) { context_param }

      it "uses context param" do
        T.reveal_type(context_param) # error: `T.untyped`
      end
    end

    describe "including with params" do
      include_examples("parameterized shared context", "test_value")

      it "has access to param_value" do
        param_value
      end
    end

    describe "including context with params" do
      include_context("another parameterized context", "context_test_value")

      it "has access to context_value" do
        context_value
      end
    end
  end
end
