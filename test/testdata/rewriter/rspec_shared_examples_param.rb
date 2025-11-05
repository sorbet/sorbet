# typed: true
# enable-experimental-rspec: true
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
  describe("single shared example with context") do
    shared_examples "my shared example" do |my_param|
      T.reveal_type(my_param) # error: `NilClass`
      let(:foo_exists) { 'bar' }

      let(:uses_my_param) {
        T.reveal_type(my_param) # error: `NilClass`
      }

      it "works for #{my_param}" do
        T.reveal_type(my_param) # error: `T.untyped`
        expect(foo_exists).to eq('bar')
        this_does_not_exist
      # ^^^^^^^^^^^^^^^^^^^ error: `this_does_not_exist` does not exist
      end
    end
  end

  describe("shared context with 2 params") do
    shared_context "context with 2 params" do |param1, param2|
      let(:value1) { param1 }
      let(:value2) { param2 }

      it "uses the params" do
        T.reveal_type(param1) # error: `T.untyped`
        T.reveal_type(param2) # error: `T.untyped`
      end
    end

    describe "including context" do
      include_context("context with 2 params", "value1", "value2")

      it "has access to values" do
        value1
        value2
      end
    end
  end

  describe("shared context with 5 params") do
    shared_context "context with 5 params" do |param1, param2, param3, param4, param5|
      let(:value1) { param1 }
      let(:value2) { param2 }
      let(:value3) { param3 }
      let(:value4) { param4 }
      let(:value5) { param5 }

      it "uses the params" do
        T.reveal_type(param1) # error: `T.untyped`
        T.reveal_type(param2) # error: `T.untyped`
        T.reveal_type(param3) # error: `T.untyped`
        T.reveal_type(param4) # error: `T.untyped`
        T.reveal_type(param5) # error: `T.untyped`
      end
    end

    describe "including context" do
      include_context("context with 5 params", "value1", "value2", "value3", "value4", "value5")

      it "has access to values" do
        value1
        value2
        value3
        value4
        value5
      end
    end
  end
end
