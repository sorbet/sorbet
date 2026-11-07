# typed: true

class Test
  def self.test_each(arg, &blk); end
end

class Foo < Test
  test_each([1, 2]) do |a|
  end

  test_each([[1, 2], [3, 4]]) do |(a, b)|
  end
end
