# @typed
require_relative "../../t"

module Foo
    class Struct
    end
end

class NotStruct
    B = Foo::Struct.new
    var = Struct.new(:foo)
end

class RealStruct
    A = Struct.new(:foo, :bar)
end
class RealStructDesugar
    class A
        declare_variables(
            :@foo => T.untyped,
            :@bar => T.untyped
        )
        attr_accessor :foo
        attr_accessor :bar
        def initialize(foo, bar)
        end
    end
end

class TwoStructs
    A = Struct.new(:foo)
    B = Struct.new(:foo)
end

class AccidentallyStruct

    # We do this in the desugarer before we've resolved the constants
    A = Struct.new(:foo, :bar)
end

class Main
    sig.returns(RealStruct::A)
    def foo
        RealStruct::A.new(2, 3)
    end
end
puts Main.new.foo
