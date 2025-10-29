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
  describe("it_behaves_like provides isolation") do
    # Define a shared example with a let variable
    shared_examples "shared behavior" do
      let(:shared_value) { 'from_shared' }

      it 'uses shared value' do
        expect(shared_value).to eq('from_shared')
      end
    end

    # Define a let variable in the outer context
    let(:shared_value) { 'from_outer' }

    # Test that it_behaves_like creates an isolated nested context
    it_behaves_like "shared behavior"

    # Test that the outer context still has its own value
    it 'outer context has its own value' do
      expect(shared_value).to eq('from_outer')
    end

    # Test nested it_behaves_like with method overriding
    describe "method isolation" do
      shared_examples "defines helper method" do
        def helper_method
          'from_shared'
        end

        it 'uses helper from shared' do
          expect(helper_method).to eq('from_shared')
        end
      end

      def helper_method
        'from_outer'
      end

      # it_behaves_like should not let the shared helper_method clobber the outer one
      it_behaves_like "defines helper method"

      it 'outer helper_method is not clobbered' do
        expect(helper_method).to eq('from_outer')
      end
    end

    # Test parameterized it_behaves_like
    shared_examples "parameterized behavior" do |expected_value|
      it 'receives parameter' do
        expect(expected_value).to eq('param_value')
      end
    end

    it_behaves_like "parameterized behavior", 'param_value'

    # Test multiple it_behaves_like calls with different parameters
    shared_examples "math behavior" do |a, b, result|
      it 'performs calculation' do
        expect(a + b).to eq(result)
      end
    end

    it_behaves_like "math behavior", 1, 2, 3
    it_behaves_like "math behavior", 5, 10, 15
  end
end
