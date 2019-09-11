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

    describe "some inner tests" do
        def inside_method
        end

        it "works inside" do
            outside_method
            inside_method
        end
    end

    before do
        @foo = T.let(3, Integer)
    end
    it 'can read foo' do
        T.assert_type!(@foo, Integer)
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
end

def junk
end
