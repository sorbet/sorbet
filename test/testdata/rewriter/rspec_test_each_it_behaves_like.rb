# typed: true
extend T::Sig

class Test
  extend T::Sig

  def self.test_each(iter, &blk); end
  def self.it(name, &blk); end
  def self.describe(name, &blk); end
  def self.it_behaves_like(name, *args); end
  def self.shared_examples(name, &blk); end

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