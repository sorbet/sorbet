# typed: strict
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
    class A < Struct
        def foo; end
        def bar; end
        def foo=(arg0); arg0; end
        def bar=(arg0); arg0; end
        sig(foo: BasicObject, bar: BasicObject).returns(A)
        def self.new(foo=nil, bar=nil)
            T.cast(nil, A)
        end
    end
end

class TwoStructs
    A = Struct.new(:foo)
    B = Struct.new(:foo)
end

class AccidentallyStruct
    class Struct
    end

    # We do this in the dsl pass before we've resolved the constants
    A = Struct.new(:foo, :bar)
end

class Main
    def main
        a = Struct.new(:foo)
        T.assert_type!(a, Class)
        T.assert_type!(a.new, Struct)
        T.assert_type!(a.new(2), Struct)

        T.assert_type!(RealStruct::A.new(2, 3), RealStruct::A)
        T.assert_type!(RealStruct::A.new(2), RealStruct::A)

        T.assert_type!(RealStructDesugar::A.new(2, 3), RealStructDesugar::A)
    end
end
puts Main.new.main
