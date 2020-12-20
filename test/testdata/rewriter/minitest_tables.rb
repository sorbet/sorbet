# typed: true

class Parent
  def foo; end
end

class Child < Parent
end

class MyTest
  extend T::Sig

  def outside_method
  end

  sig do
    type_parameters(:U)
    .params(arg: T::Enumerable[T.type_parameter(:U)], blk: T.proc.params(arg0: T.type_parameter(:U)).void)
    .void
  end
  def self.test_each(arg, &blk); end

  sig do
    type_parameters(:K, :V)
    .params(
      arg: T::Hash[T.type_parameter(:K), T.type_parameter(:V)],
      blk: T.proc.params(arg0: T.type_parameter(:K), arg1: T.type_parameter(:V)).void,
    ).void
  end
  def self.test_each_hash(arg, &blk); end

  sig {params(name: String, blk: T.proc.void).void}
  def self.it(name, &blk); end

  test_each [Parent.new, Child.new] do |value|
    it "works with instance methods" do
      puts value.foo
      outside_method
      T.reveal_type(value) # error: type: `Parent`
    end
  end

  CONST_LIST = [Parent.new, Child.new]
  test_each CONST_LIST do |value|
    it "succeeds with a constant list" do
      puts value.foo
      T.reveal_type(value) # error: type: `T.untyped`
    end
  end

  ANOTHER_CONST_LIST = T.let([Parent.new, Child.new], T::Array[Parent])
  test_each ANOTHER_CONST_LIST do |value|
    it "succeeds with a typed constant list" do
      puts value.foo
      T.reveal_type(value) # error: type: `Parent`
    end
  end

  local = [Parent.new, Child.new]
  test_each local do |value|
    it "succeed with a local variable but cannot type it" do
      puts value.foo
      T.reveal_type(value) # error: type: `T.untyped`
    end
  end

  test_each [Parent.new, Child.new] do |x|
    y = x # error: Only valid `it`-blocks can appear within `test_each`
  end

  test_each_hash({}) do |k, v|
    y = k + v # error: Only valid `it`-blocks can appear within `test_each_hash`
  end

  test_each CONST_LIST do |value|
    x = value.foo  # error: Only valid `it`-blocks can appear within `test_each`
    it "fails with non-it statements" do
      puts x
    end
  end

  test_each ["foo", 5, {x: false}] do |v|
    it "handles lists with several types" do
      T.reveal_type(v) # error: Revealed type: `T.any(String, Integer, T::Hash[Symbol, FalseClass])`
    end
  end


  test_each [1, 2, 3] do |k, v| # error: Wrong number of parameters for `test_each` block
    it "does not handle more than one argument" do
    end
  end

  test_each [1, 2, 3] do # error: Wrong number of parameters for `test_each` block
    it "does not handle more than one argument" do
    end
  end

  test_each_hash({foo: 1, bar: 2, baz: 3}) do |k, v|
    it "handles lists with several types" do
      T.reveal_type(k) # error: Revealed type: `Symbol`
      T.reveal_type(v) # error: Revealed type: `Integer`
    end
  end

  test_each_hash([1, 2, 3]) do |k, v| # error: Expected `T::Hash[T.type_parameter(:K), T.type_parameter(:V)]`
    it "fails to typecheck with non-hash arguments to `test_each-hash`" do
      puts k, v
    end
  end

  test_each_hash({foo: 1, bar: 2}) do |x| # error: Wrong number of parameters for `test_each_hash` block
    it "does not handle more than one argument" do
    end
  end

end
