# typed: true
extend T::Sig

class Test
  extend T::Sig

  def self.test_each(iter, &blk); end

  shared_examples "common behavior" do
    it "works" do
      puts "shared example"
    end
  end

  test_each([1, 2, 3]) do |value|
    describe "for value #{value}" do
      it_behaves_like "common behavior"
      
      it "has the value" do
        T.reveal_type(value) # error: Revealed type: `Integer`
      end
    end
  end
end

# Define minimal RSpec for testing
module RSpec
  module Core
    class ExampleGroup
      def self.describe(name, &block); end
      def expect(*args); end
      def eq(*args); end
    end
  end
  
  def self.describe(name, &block)
    Core::ExampleGroup.describe(name, &block)
  end
  
  def self.shared_examples(name, &block); end
end
