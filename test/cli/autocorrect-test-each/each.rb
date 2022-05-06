# typed: true
extend T::Sig

class MyTest
  # Hack in things that look Minitest-ish.
  def self.describe(name, &blk); end
  def self.it(name, &blk); end
  def self.test_each(val, &blk); end


  # This is actually an OK use of `each`.
  [1, 2].each do |x|
    describe "weird case #{x}" do
      it "doesn't call out to test_method" do
        z = x.to_s
      end
    end
  end

  describe "array test" do
    [1, 2].each do |x|
      it "asdf #{x}" do
        z = test_method(x)
      end
    end
  end

  describe "nested array test" do
    [[1, 2], [3, 4]].each do |(x, y)|
      it "asdf #{x} #{y}" do
        z = test_method(x) + test_method(y)
      end
    end
  end

  describe "hash test" do
    {1 => 2, 3 => 4}.each do |x, y|
      it "asdf #{x} #{y} hash" do
        z = test_method(x) + test_method(y)
      end
    end
  end

  describe "hash test one arg" do
    {1 => 2, 3 => 4}.each do |a|
      it "asdf #{a} hash" do
        # We should not complain about this use of `each`.
        z = 0
        a.each do
          z += test_method(a)
        end
      end
    end
  end

  describe "test_each should work" do
    test_each([1,2]) do |x|
      it "asdf #{x} test_each" do
        z = test_method(x)
      end
    end
  end

  def test_method(x); end
end
