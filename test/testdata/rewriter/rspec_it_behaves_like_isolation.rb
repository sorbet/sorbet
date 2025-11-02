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
    extend T::Sig

    # Define a shared example with a let variable
    shared_examples "shared behavior" do
      extend T::Sig

      sig { returns(String) }
      let(:shared_value) { 'from_shared' }

      it 'uses shared value' do
        result = shared_value
        T.reveal_type(result) # error: Revealed type: `String`
        expect(result).to eq('from_shared')
      end
    end

    # Define a let variable in the outer context with a different type
    sig { returns(Integer) }
    let(:shared_value) { 100 }

    # Test that it_behaves_like creates an isolated nested context
    it_behaves_like "shared behavior"

    # Test that the outer context still has its own value and type (not clobbered by shared behavior)
    it 'outer context has its own value' do
      result = shared_value
      T.reveal_type(result) # error: Revealed type: `Integer`
      expect(result).to eq(100)
    end

    # Test nested it_behaves_like with method overriding
    describe "method isolation" do
      extend T::Sig

      shared_examples "defines helper method" do
        extend T::Sig

        sig { returns(String) }
        def helper_method
          'from_shared'
        end

        it 'uses helper from shared' do
          result = helper_method
          T.reveal_type(result) # error: Revealed type: `String`
          expect(result).to eq('from_shared')
        end
      end

      sig { returns(Integer) }
      def helper_method
        42
      end

      # it_behaves_like should not let the shared helper_method clobber the outer one
      it_behaves_like "defines helper method"

      it 'outer helper_method is not clobbered' do
        result = helper_method
        T.reveal_type(result) # error: Revealed type: `Integer`
        expect(result).to eq(42)
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
