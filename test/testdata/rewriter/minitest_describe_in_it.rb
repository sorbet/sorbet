# typed: true

class ClassA
  it 'first_test' do
    describe 'BAR' do
    end
  end

  describe 'OUTER' do
    it 'second_test' do
      describe 'QUX' do
      end
    end
  end
end

module ModuleB
  it 'first_test' do
    describe 'BAR' do
    end
  end

  describe 'OUTER' do
    it 'second_test' do
      describe 'QUX' do
      end
    end
  end
end

class StressTest
  def self.describe(desc, &block)
  end

  it "foo" do
    one = 1
    Class.new do
      one.times do
        describe "weird context" do # error: Method `describe` does not exist on `T::Class[Object]`
          "" + 1 # error: Expected `String` but found `Integer(1)` for argument `arg0`
        end
      end
    end
  end

  it "bar" do
    one = 1
    # Note: The `Class.new` rewriter is not running on any classes inside a test.
    # That might be nice to change at some point, but it would be weird because it would amount to class definitions inside method bodies.
    MyClass = Class.new do
      one.times do
        describe "weird context" do # error: Method `describe` does not exist on `T::Class[Object]`
          "" + 1 # error: Expected `String` but found `Integer(1)` for argument `arg0`
        end
      end
    end
  end
end
