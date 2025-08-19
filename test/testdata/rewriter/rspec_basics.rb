# typed: true

class RSpecBasicsTest
  # Test traditional describe syntax (should still work)
  describe 'traditional describe syntax' do
    it 'still works without RSpec receiver' do
      puts 'traditional test'
    end
  end

  # Test context as alias for describe
  context 'using context instead of describe' do
    it 'context should work like describe' do
      puts 'context test'
    end

    context 'nested contexts' do
      it 'should handle nesting' do
        puts 'nested context test'
      end
    end
  end

  # Test xit (pending/skipped tests) - only named ones for now
  describe 'pending tests with xit' do
    it 'regular test' do
      puts 'regular test runs'
    end

    xit 'pending test with description' do
      puts 'this should be skipped'
    end
  end

  # Test mixed describe and context
  describe 'mixed syntax' do
    context 'inside describe' do
      it 'regular it block' do
        puts 'mixed syntax test'
      end

      xit 'pending in context' do
        puts 'pending test'
      end
    end

    describe 'nested describe' do
      it 'should work fine' do
        puts 'nested traditional in describe'
      end
    end
  end

  module Foo
    class Baz
    end
  end

  module Bar
    class Baz
    end
  end

  module RSpec
    module Core
      class ExampleGroup
      end
    end
  end

  describe Foo::Baz do
    it 'should work fine' do
      puts 'nested traditional in describe'
    end
  end

  RSpec.describe Bar::Baz do
    it 'should work fine' do
      puts 'nested traditional in describe'
    end
  end
end
