# typed: true

class RSpecExpectTest
  extend T::Sig

  # Test expect blocks (simpler than shared examples for now)
  describe 'expect functionality' do
    it 'can use expect blocks' do
      expect { puts 'testing expect block' } # error: Method `expect` does not exist
      puts 'test completed'
    end

    it 'can use change blocks' do 
      change { puts 'testing change block' } # error: Method `change` does not exist
      puts 'change test completed'
    end
  end

  # Test nested expect usage
  describe 'nested expect usage' do
    context 'with expect in context' do
      it 'works in nested structure' do
        expect { puts 'nested expect' } # error: Method `expect` does not exist
        puts 'nested test'
      end
    end
  end
end