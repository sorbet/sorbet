# typed: true
require 'minitest/autorun'
require_relative 'gems/sorbet-runtime/lib/sorbet-runtime'
extend T::Sig

# Test combining 'it' block parameter with Minitest's 'it' method and test_each/test_each_hash

class Minitest::Spec
  def self.test_each(arg, &blk)
    arg.each(&blk)
  end

  def self.test_each_hash(arg, &blk)
    arg.each(&blk)
  end
end

class MinitestItParamTest < Minitest::Spec
  # Basic Minitest 'it' method with 'it' block parameter
  it "should process items" do
    # Using 'it' as block parameter inside minitest 'it' block
    result = [1, 2, 3].map { it * 2 }
    T.reveal_type(result) # error: Revealed type: `T::Array[Integer]`
  end

  # Local variable named 'it' should shadow block parameter
  it "should handle local variable shadowing" do
    it = 10

    # This 'it' refers to local variable, not block parameter
    result = [1, 2, 3].map { |x| x + it }
    T.reveal_type(result) # error: Revealed type: `T::Array[Integer]`
  end

  # Nested blocks with 'it'
  it "should handle nested blocks" do
    outer = [[1, 2], [3, 4]].map do
      it.map { it * 2 }
    end
    T.reveal_type(outer) # error: Revealed type: `T::Array[T::Array[Integer]]`
  end
  # Using test_each with 'it' block parameter in describe blocks
  test_each([[1, 2, 3], [4, 5, 6]]) do |array_data|
    describe "with array #{array_data}" do
      it "processes items with it parameter" do
        # Using 'it' block parameter to process array
        result = array_data.map { it * 2 }

        T.reveal_type(result) # error: Revealed type: `T::Array[Integer]`

        # Using 'it' parameter in select
        large = array_data.select { it > 3 }

        T.reveal_type(large) # error: Revealed type: `T::Array[Integer]`
      end
    end
  end

  # Using test_each_hash with 'it' block parameter
  test_each_hash({
    small: [1, 2],
    large: [10, 20, 30],
  }) do |key, values|
    describe "test_each_hash with #{key}" do
      it "sums correctly" do
        sum = values.reduce(0) { |acc, x| acc + x }

        # Using 'it' parameter in another block
        doubled = values.map { it * 2 }

        T.reveal_type(doubled) # error: Revealed type: `T.untyped`
      end
    end
  end

  # Combining nested 'it' blocks in test_each context
  test_each([[[1, 2], [3, 4]]]) do |nested_arrays|
    describe "with nested arrays" do
      it "handles nested blocks" do
        # Outer block with 'it' parameter
        result = nested_arrays.map do
          it.map { it * 10 }
        end

        T.reveal_type(result) # error: Revealed type: `T::Array[T::Array[Integer]]`
      end
    end
  end

  # Testing 'it' parameter with proc in test_each
  test_each([[1, 2, 3]]) do |numbers|
    describe "with proc" do
      it "works with Proc#call" do
        # Proc with 'it' parameter
        multiplier = Proc.new { it * 5 }

        # Using 'it' parameter in map
        result = numbers.map { multiplier.call(it) }

        T.reveal_type(result) # error: Revealed type: `T::Array[T.untyped]`
      end
    end
  end

  # Local variable named 'it' should shadow block parameter
  test_each([[7, 8, 9]]) do |data|
    describe "with shadowing" do
      it "handles local variable shadowing" do
        it = 100

        # This 'it' is the local variable, not block parameter
        result = data.map { |x| x + it }

        T.reveal_type(result) # error: Revealed type: `T::Array[Integer]`
      end
    end
  end

  # Testing 'it' with method calls
  test_each([[1, 2, 3, 4, 5]]) do |nums|
    describe "with method calls" do
      it "calls methods on it parameter" do
        # Using 'it' parameter with method calls
        evens = nums.select { it.even? }

        strings = nums.map { it.to_s }

        T.reveal_type(evens) # error: Revealed type: `T::Array[Integer]`
        T.reveal_type(strings) # error: Revealed type: `T::Array[String]`
      end
    end
  end
end
