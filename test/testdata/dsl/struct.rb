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
    KeywordInit = Struct.new(:foo, :bar, keyword_init: true)
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

  MyKeywordInitStruct = Struct.new(:x, keyword_init: true) do
    include MyMixin
    self.new.x
    self.new.foo
    self.new(1, 2) # error: Too many arguments provided for method `MixinStruct::MyKeywordInitStruct.new`. Expected: `0`, got: `2`
    self.new(giberish: 1) # error: Unrecognized keyword argument `giberish` passed for method `MixinStruct::MyKeywordInitStruct.new`
  end

  MyKeywordInitStruct.new(1, 2) # error: Too many arguments provided for method `MixinStruct::MyKeywordInitStruct.new`. Expected: `0`, got: `2`
  MyKeywordInitStruct.new(giberish: 1) # error: Unrecognized keyword argument `giberish` passed for method `MixinStruct::MyKeywordInitStruct.new`
  MyStruct.new.x
  MyStruct.new.foo
end

class BadUsages
  A = Struct.new # error: Not enough arguments provided for method `Struct.new`. Expected: `1+`, got: `0`
  B = Struct.new(giberish: 1) # error: `{giberish: Integer(1)}` doesn't match `T.any(Symbol, String)` for argument `arg0`
  C = Struct.new(keyword_init: true) # error: `{keyword_init: TrueClass}` doesn't match `T.any(Symbol, String)` for argument `arg0`
  local = true
  D = Struct.new(keyword_init: local) # error: `{keyword_init: TrueClass}` doesn't match `T.any(Symbol, String)` for argument `arg0`
  E = Struct.new(:a, keyword_init: local) # we run too early in to be able to support this
end

class Main
    def main
        a = Struct.new(:foo)
        T.assert_type!(a, Class)
        T.assert_type!(a.new, Struct)
        T.assert_type!(a.new(2), Struct)

        T.assert_type!(RealStruct::A.new(2, 3), RealStruct::A)
        T.assert_type!(RealStruct::A.new(2), RealStruct::A)

        T.assert_type!(RealStruct::KeywordInit.new, RealStruct::KeywordInit)
        T.assert_type!(RealStruct::KeywordInit.new(foo: 1), RealStruct::KeywordInit)
        T.assert_type!(RealStruct::KeywordInit.new(foo: 2, bar: 3), RealStruct::KeywordInit)
        RealStruct::KeywordInit.new(1, 2) # error: Too many arguments provided for method `RealStruct::KeywordInit.new`. Expected: `0`, got: `2`

        T.assert_type!(RealStructDesugar::A.new(2, 3), RealStructDesugar::A)
    end
end
puts Main.new.main
