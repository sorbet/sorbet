# typed: true

class Abstract
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.params(req: Integer, opt: Integer, kwreq: Integer, kwopt: Integer).returns(Integer) }
  def foo(req, opt=1, kwreq:, kwopt: 2); end
end

module NonMatchingTests
  class Okay < Abstract
    sig { override.params(req: Integer, opt: Integer, kwreq: Integer, kwopt: Integer).returns(Integer) }
    def foo(req, opt=1, kwreq:, kwopt: 2); 55; end
  end

  class BadPos < Abstract
    sig { override.params(req: String, opt: Integer, kwreq: Integer, kwopt: Integer).returns(Integer) }
    def foo(req, opt=1, kwreq:, kwopt: 2); 55; end # error: Parameter `req` of type `String` not compatible with type of abstract method `Abstract#foo`
  end

  class BadOptPos < Abstract
    sig { override.params(req: Integer, opt: String, kwreq: Integer, kwopt: Integer).returns(Integer) }
    def foo(req, opt='foo', kwreq:, kwopt: 2); 55; end # error: Parameter `opt` of type `String` not compatible with type of abstract method `Abstract#foo`
  end

  class BadKw < Abstract
    sig { override.params(req: Integer, opt: Integer, kwreq: String, kwopt: Integer).returns(Integer) }
    def foo(req, opt=1, kwreq:, kwopt: 2); 55; end # error: Keyword parameter `kwreq` of type `String` not compatible with type of abstract method `Abstract#foo`
  end

  class BadKwOpt < Abstract
    sig { override.params(req: Integer, opt: Integer, kwreq: Integer, kwopt: String).returns(Integer) }
    def foo(req, opt=1, kwreq:, kwopt: 'bar'); 55; end # error: Keyword parameter `kwopt` of type `String` not compatible with type of abstract method `Abstract#foo`
  end

  class BadReturn < Abstract
    sig { override.params(req: Integer, opt: Integer, kwreq: Integer, kwopt: Integer).returns(String) }
    def foo(req, opt=1, kwreq:, kwopt: 2); 'baz'; end # error: Return type `String` does not match return type of abstract method `Abstract#foo`
  end
end

module VarianceTests
  # C <: B <: A
  class A; end
  class B < A; end
  class C < B; end

  class Abstract
    extend T::Sig
    extend T::Helpers
    abstract!

    sig { abstract.params(arg: B).returns(B) }
    def bar(arg); end
  end

  class GoodOverride < Abstract
    sig { override.params(arg: B).returns(B) }
    def bar(arg); B.new; end
  end

  # methods are contravariant in their arguments, so an implementation
  # could reasonably accept a supertype of the declared interface type
  class ArgWidening < Abstract
    sig { override.params(arg: A).returns(B) }
    def bar(arg); B.new; end
  end

  # but an implementation cannot reasonably accept a subtype
  class ArgNarrowing < Abstract
    sig { override.params(arg: C).returns(B) }
    def bar(arg); B.new; end # error: Parameter `arg` of type `VarianceTests::C` not compatible with type of abstract method `VarianceTests::Abstract#bar`
  end

  # methods are covariant in their return types, so an implementation
  # could reasonably return a subtype of the declared interface type
  class RetNarrowing < Abstract
    sig { override.params(arg: B).returns(C) }
    def bar(arg); C.new; end
  end

  # but an implementation cannot reasonably return a supertype
  class RetWidening < Abstract
    sig { override.params(arg: B).returns(A) }
    def bar(arg); A.new; end # error: Return type `VarianceTests::A` does not match return type of abstract method `VarianceTests::Abstract#bar`
  end

end
