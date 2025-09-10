# typed: true

class ClassA
  it 'first_test' do
    describe 'BAR' do # error: Method `describe` does not exist on `T.class_of(ClassA)`
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
    describe 'BAR' do # error: Method `describe` does not exist on `T.class_of(ModuleB)`
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
        describe "weird context" do
          "" + 1 # error: Expected `String` but found `Integer(1)` for argument `arg0`
        end
      end
    end
  end

  it "bar" do
    one = 1
    MyClass = Class.new do
      one.times do
        describe "weird context" do
          "" + 1 # error: Expected `String` but found `Integer(1)` for argument `arg0`
        end
      end
    end
  end
end
