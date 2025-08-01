# typed: true

class RSpecIncludeContextTest
  extend T::Sig

  # Test include_context functionality (simplified)
  describe 'include context usage' do
    it 'can reference include_context' do
      include_context 'some shared context' # error: Unable to resolve constant `<shared_examples 'some shared context'>`
      puts 'include context test'
    end

    it 'can reference include_examples' do  
      include_examples 'some shared examples' # error: Method `include_examples` does not exist
      puts 'include examples test' 
    end

    it 'can reference it_behaves_like' do
      it_behaves_like 'some shared behavior' # error: Method `it_behaves_like` does not exist
      puts 'it behaves like test'
    end
  end
end