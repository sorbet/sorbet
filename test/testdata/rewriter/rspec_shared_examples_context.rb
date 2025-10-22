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
    shared_examples "my shared example" do
      let(:foo_exists) { 'bar' }

      context 'context' do
        it 'works' do
          expect(foo_exists).to eq('bar')
        # ^^^^^^ error: `expect` does not exist
          #      ^^^^^^^^^^ error: `foo_exists` does not exist
          #                     ^^ error: `eq` does not exist
          this_does_not_exist
        # ^^^^^^^^^^^^^^^^^^^ error: `this_does_not_exist` does not exist
        end
      end
    end
  end
end
