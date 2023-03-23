# typed: true
require_relative "../../t"

module Foo
    class Data
    end
end

class NotData
    B = T.let(Foo::Data.new, Foo::Data)
    var = Data.define(:foo)
end

class RealData
    A = Data.define(:foo, :bar)
end

class RealDataDesugar
    class A < Data
        extend T::Sig
        def foo; end
        def bar; end
        sig {params(foo: BasicObject, bar: BasicObject).returns(A)}
        def self.new(foo=nil, bar=nil)
            T.cast(nil, A)
        end
    end
end

class TwoDatas
    A = Data.define(:foo)
    B = Data.define(:foo)
end

class AccidentallyData
    class Data
      def self.define; end
    end

    # We do this in the Rewriter pass before we've typeAlias the constants
    A = Data.define(:foo, :bar)
end

class InvalidMember
  A = Data.define(:foo=) # error: Data member `foo=` cannot end with an equal
end

class MixinData
  module MyMixin
    def foo; end
  end

  MyData = Data.define(:x) do
    include MyMixin
    self.new(1).x
    self.new(1).foo
  end

  MyData.new(1).x
  MyData.new(1).foo
end

class BadUsages
  A = Data.define # error: Not enough arguments provided for method `Data.define`. Expected: `1+`, got: `0`
  B = Data.define(giberish: 1)
  #               ^^^^^^^^^^^ error: Expected `T.any(Symbol, String)` but found `{giberish: Integer(1)}` for argument `arg0`

  C = Data.define(:c)
  c_data = C.new(1)
  c_data.c = 6 # error: Method `c=` does not exist on `BadUsages::C`
end

class Main
    def main
        a = Data.define(:foo)
        # a.is_a?(Data) is actually false, because `Data.define` dynamically
        # allocates and returns a class object for this struct, but we don't
        # have a great way to model that statically in the case where the
        # result is assigned to a local variable, not a constant.
        T.assert_type!(a, Data)
        T.assert_type!(a.new(2), Data)

        # This should raise a "Not enough arguments" error, but it doesn't because the rewriter
        # currently doesn't know how to typecheck when LHS is an ident instead of a constant.
        # Is this okay?
        a.new

        T.assert_type!(RealData::A.new(2, 3), RealData::A)

        T.assert_type!(RealDataDesugar::A.new(2, 3), RealDataDesugar::A)
    end
end
puts Main.new.main

class FullyQualifiedDataUsages
  Foo = Data.define(:a)
  Bar = ::Data.define(:a)
  Baz = ::Foo::Data.new

  Foo.new(1).a
  Bar.new(1).a
end
