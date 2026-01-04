# typed: true
class MyTest
    def outside_method
    end

    it "works outside" do
        outside_method
    end

    it "allows constants inside of IT" do
      CONST = 10
    end

    it "allows let-ed constants inside of IT" do
      C2 = T.let(10, Integer)
    end

    it "allows path constants inside of IT" do
      C3 = Mod::C
      C3.new
    end

    it "finds errors in the test name: #{bad_variable}" do # error: Method `bad_variable` does not exist on `T.class_of(MyTest)`
    end

    describe "some inner tests" do
        def inside_method
        end

        it "works inside" do
            outside_method
            inside_method
        end
    end

    def instance_helper; end

    before do
        @foo = T.let(3, Integer)
        instance_helper
    end

    after do
        @foo = nil # error: Expected `Integer` but found `NilClass` for field
        instance_helper
    end

    it 'can read foo' do
        T.assert_type!(@foo, Integer)
        instance_helper
    end

    def self.random_method
    end

    random_method do
        @random_method_ivar = T.let(3, Integer) # error: The instance variable `@random_method_ivar` must be declared inside `initialize`
    end

    describe Object do
        it Object do
        end
        it Object do
        end
    end

    def self.it(*args)
    end
    it "ignores methods without a block"

    junk.it "ignores non-self calls" do
        junk
    end

    describe "a non-ideal situation" do
      it "contains nested describes" do
        describe "nobody should write this but we should still parse it" do
        end
      end
    end

    it do
      puts("anonymous it blocks")
    end

    # Minitest describe should only accept exactly 1 argument
    # Multiple arguments should not be transformed by the rewriter
    describe "test", :metadata do # error: Method `describe` does not exist on `T.class_of(MyTest)`
      it "should not work" do
      end
    end
end

describe 'extends T::Sig' do
  extend T::Sig

  sig { returns(Integer) }
  def example = 0

  it 'calls example' do
    res = example
    T.reveal_type(res) # error: `Integer`
  end
end

def junk
end


module Mod
  class C
  end
end
