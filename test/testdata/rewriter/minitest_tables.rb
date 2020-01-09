# typed: true

class A
  def foo; end
end
class B < A
end

class MyTest
  def outside_method
  end

  def self.test_each(arg, &blk); end
  def self.it(name, &blk); end

  test_each [A.new, B.new] do |value|
    it "works with instance methods" do
      puts value.foo
      outside_method
      T.reveal_type(value) # error: type: `A`
    end
  end

  CONST_LIST = [A.new, B.new]
  test_each CONST_LIST do |value|
    it "succeeds with a constant list" do
      puts value.foo
      T.reveal_type(value) # error: type: `T.untyped`
    end
  end

  ANOTHER_CONST_LIST = T.let([A.new, B.new], T::Array[A])
  test_each ANOTHER_CONST_LIST do |value|
    it "succeeds with a typed constant list" do
      puts value.foo
      T.reveal_type(value) # error: type: `A`
    end
  end

  local = [A.new, B.new]
  test_each local do |value|
    it "succeed with a local variable but cannot type it" do
      puts value.foo
      T.reveal_type(value) # error: type: `T.untyped`
    end
  end

  test_each [A.new, B.new] do |x|
    y = x # error: Only valid `it`-blocks can appear within `test_each`
  end

  test_each [A.new, B.new] do |value|
    x = value.foo  # error: Only valid `it`-blocks can appear within `test_each`
    it "fails with non-it statements" do
      puts x
    end
  end

  test_each ["foo", 5, {x: false}] do |v|
    it "handles lists with several types" do
      T.reveal_type(v) # error: Revealed type: `T.any(String, Integer, T::Hash[T.untyped, T.untyped])`
    end
  end
end
