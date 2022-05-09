# typed: true

class MyTest
  def self.test_each_hash(hash, &blk)
  end

  def self.it(name, &blk)
  end
end

class A < MyTest
  test_each_hash(left: -1, right: 1) do |key, value|
    #            ^^^^^^^^^^^^^^^^^^ error: `test_each_hash` expects a single `Hash` argument, not keyword args
    it 'hello' do
      p(key)
      p(value)
    end
  end
end
