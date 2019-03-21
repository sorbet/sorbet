# typed: true
require_relative "../../t"

module Foo
    class Struct
    end
end

class NotStruct
    B = T.let(Foo::Struct.new, Foo::Struct)
    var = Struct.new(:foo)
end

class RealStruct
    A = Struct.new(:foo, :bar)
end
class RealStructDesugar
    class A < Struct
        extend T::Sig
        def foo; end
        def bar; end
        def foo=(arg0); arg0; end
        def bar=(arg0); arg0; end
        sig {params(foo: BasicObject, bar: BasicObject).returns(A)}
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

    # We do this in the dsl pass before we've typeAlias the constants
    A = Struct.new(:foo, :bar)
end

class MixinStruct
  module MyMixin
    def foo; end
  end

  MyStruct = Struct.new(:x) do
    include MyMixin
    self.new.x
    self.new.foo
  end

  MyStruct.new.x
  MyStruct.new.foo
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
