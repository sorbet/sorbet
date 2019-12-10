# typed: true

class A
  def foo; end
end
class B < A
end

class MyTest
  def outside_method
  end

  [A.new, B.new].each do |value|
    puts value.foo
    it "works outside" do
      puts value.foo
      outside_method
    end
  end

  [A.new, B.new].each do |value|
    puts value.foo
    describe "some inner tests" do
      def inside_method
      end

      it "works inside" do
        T.reveal_type(value) # error: Revealed type: A
        puts value.foo
        outside_method
        inside_method
      end
    end
  end

  ["foo", 5, {x: false}].each do |v|
    it "handles lists with several types" do
      T.reveal_type(v) # error: Revealed type: T.any(String, Integer, T::Hash[T.untyped, T.untyped])
    end
  end

  [1, 2].each do |x|
    [3, 4].each do |y|
      it "handles nested eaches" do
        puts (x + y)
      end
    end
  end
end
