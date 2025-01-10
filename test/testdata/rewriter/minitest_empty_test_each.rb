# typed: true

class Test
  extend T::Sig
  def self.test_each(arg, &blk); end
  def self.it(name, &blk); end
  def self.describe(name, &blk); end
end

class Foo < Test
  # The unclosed `do` block here should be a recoverable parse error.
  test_each([[1, 2], [3,4]]) do |(a,b)|
                           # ^^ error: Hint: this "do" token

  it "it block 1" do
  end

  it "it block 2" do
  end
end # error: unexpected token "end of file"
