# typed: true

class Parent
  def foo; end
end

class Child < Parent
end

class NoShow < BasicObject
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
    y = x # error: Only valid `it`, `before`, `after`, and `describe` blocks can appear within `test_each`
  end

  test_each_hash({}) do |k, v|
    y = k + v # error: Only valid `it`, `before`, `after`, and `describe` blocks can appear within `test_each_hash`
  end

  test_each CONST_LIST do |value|
    x = value.foo  # error: Only valid `it`, `before`, `after`, and `describe` blocks can appear within `test_each`
    it "fails with non-it statements" do
      puts x
    end
  end

  test_each ["foo", 5, {x: false}] do |v|
    it "handles lists with several types" do
      T.reveal_type(v) # error: Revealed type: `T.any(Integer, String, T::Hash[T.untyped, T.untyped])`
    end
  end


  test_each [1, 2, 3] do |k, v|
    it "blocks get padded with NilClass arguments" do
      T.reveal_type(k) # error: Revealed type: `Integer`
      T.reveal_type(v) # error: Revealed type: `NilClass`
    end

    it "multiple it blocks are ok with multiple arguments" do
      T.reveal_type(k) # error: Revealed type: `Integer`
      T.reveal_type(v) # error: Revealed type: `NilClass`
    end
  end

  test_each [1, 2, 3] do |(k, v)|
    it "destructured blocks get padded with NilClass arguments" do
      T.reveal_type(k) # error: Revealed type: `Integer`
      T.reveal_type(v) # error: Revealed type: `NilClass`
    end

    it "multiple it blocks are ok with multiple destructured arguments" do
      T.reveal_type(k) # error: Revealed type: `Integer`
      T.reveal_type(v) # error: Revealed type: `NilClass`
    end
  end

  test_each [[1, ['hi', false]], [2, ['bye', true]]] do |i, (s,b), (x, y)|
    it "handles mixed destructuring and positional arguments" do
      T.reveal_type(i) # error: Revealed type: `Integer`
      T.reveal_type(s) # error: Revealed type: `String`
      T.reveal_type(b) # error: Revealed type: `T::Boolean`
      T.reveal_type(x) # error: Revealed type: `NilClass`
      T.reveal_type(y) # error: Revealed type: `NilClass`
    end

    it "multiple it blocks are ok with mixed argument styles" do
      T.reveal_type(i) # error: Revealed type: `Integer`
      T.reveal_type(s) # error: Revealed type: `String`
      T.reveal_type(b) # error: Revealed type: `T::Boolean`
    end
  end

  CONST_LIST_TUPLE = [[1,'a'], [2,'b']]
  test_each CONST_LIST_TUPLE do |i, s|
    it "succeeds with a constant list of tuples" do
      T.reveal_type(i) # error: type: `T.untyped`
      T.reveal_type(s) # error: type: `T.untyped`
    end
  end

  test_each CONST_LIST_TUPLE do |(i, s)|
    it "succeeds with a constant list of tuples and destructuring" do
      T.reveal_type(i) # error: type: `T.untyped`
      T.reveal_type(s) # error: type: `T.untyped`
    end
  end

  ANOTHER_CONST_LIST_TUPLE = T.let([[1,'a'], [2,'b']], T::Array[[Integer, String]])
  test_each ANOTHER_CONST_LIST_TUPLE do |i, s|
    it "succeeds with a typed constant tuple list" do
      T.reveal_type(i) # error: type: `Integer`
      T.reveal_type(s) # error: type: `String`
    end
  end

  test_each ANOTHER_CONST_LIST_TUPLE do |(i, s)|
    it "succeeds with a typed constant tuple list with destructuring" do
      T.reveal_type(i) # error: type: `Integer`
      T.reveal_type(s) # error: type: `String`
    end
  end

  local_tuple = [[1,'a'], [2,'b']]
  test_each local_tuple do |i,s|
    it "succeed with local variables but cannot type them" do
      T.reveal_type(i) # error: type: `T.untyped`
      T.reveal_type(s) # error: type: `T.untyped`
    end
  end

  test_each local_tuple do |(i,s)|
    it "succeed with destructured local variables but cannot type them" do
      T.reveal_type(i) # error: type: `T.untyped`
      T.reveal_type(s) # error: type: `T.untyped`
    end
  end

  test_each [1, 2, 3] do # error: Wrong number of parameters for `test_each` block
    it "does not handle zero argument blocks" do
    end
  end

  # We don't allow manual destructuring currently.
  test_each [[1,'a'], [2,'b']] do |value|
    i, s = value # error: Only valid `it`, `before`, `after`, and `describe` blocks can appear within `test_each`
    it "rejects manual destructuring of the list argument" do
      T.reveal_type(i) # error: type: `NilClass`
      T.reveal_type(s) # error: type: `NilClass`
    end
  end

  # String interpolation here is fine, but also broken, as we will use the textual representation of the string
  # interpolation as the name of the test. This test will end up with a name like:
  #
  # def <it '::<Magic>.<string-interpolate>("color ", color, " which is not purple is not number 1 ", ns)'><<todo method>>(&<blk>)
  #
  # As a result this masks an error: `ns` is used in a context that would call a `to_s` method that does not exist on
  # it.
  test_each([['red', NoShow.new], ['blue', NoShow.new]]) do |(color, ns)|
    it "color #{color} which is not purple is not number 1 #{ns}" do
      T.reveal_type(color) # error: type: `String`
      T.reveal_type(ns)  # error: type: `NoShow`
    end
  end

  test_each_hash({foo: 1, bar: 2, baz: 3}) do |k, v|
    it "handles lists with several types" do
      # we don't decay literal hash types like we do for arrays, so
      # these will still be untyped here
      T.reveal_type(k) # error: Revealed type: `T.untyped`
      T.reveal_type(v) # error: Revealed type: `T.untyped`
    end
  end

  test_each_hash([1, 2, 3]) do |k, v|
    #            ^^^^^^^^^ error: Expected `T::Hash[T.type_parameter(:K), T.type_parameter(:V)]`
    #                              ^ error: This code is unreachable
    it "fails to typecheck with non-hash arguments to `test_each-hash`" do
      puts k, v
    end
  end

  test_each_hash({foo: 1, bar: 2}) do |x| # error: Wrong number of parameters for `test_each_hash` block
    it "does not handle more than one argument" do
    end
  end

end
