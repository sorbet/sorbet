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
  describe("single shared example with context") do
    shared_examples "my shared example" do |my_param|
      T.reveal_type(my_param) # error: `NilClass`
      let(:foo_exists) { 'bar' }

      let(:uses_my_param) {
        T.reveal_type(my_param) # error: `NilClass`
      }

      it "works for #{my_param}" do
        T.reveal_type(my_param) # error: `NilClass`
        expect(foo_exists).to eq('bar')
        this_does_not_exist
      # ^^^^^^^^^^^^^^^^^^^ error: `this_does_not_exist` does not exist
      end
    end
  end
end
