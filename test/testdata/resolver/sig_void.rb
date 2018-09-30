# typed: true

class Main
    extend T::Helpers

    sig {void}
    def initialize
    end

    def foo
    end

    sig {void}
    def self.voider
    end

    sig {void.returns(Integer)} # error: Don't use both .returns() and .void
    def self.void_returns
        3
    end

    sig {returns(Integer).void} # error: Don't use both .returns() and .void
    def self.returns_void
        3
    end

    sig {void}
    def self.missing_arg(my_arg) # error: Type not specified for argument `my_arg`
    end
end

Main.new.foo
Main.voider.bad # error: Can not call method `bad` on void type
Main.voider.equal?(3) # error: Can not call method `equal?` on void type
