# typed: strict

extend T::Sig

module Wrong
     # ^^^^^ error: File belongs to package `Root::Nested` but defines a constant that does not match this namespace
  class Inside; end
end

Root::Nested::Foo::Bar = nil
::Allowed::TopLevel = nil

  NotAllowed::Foo::Bar = nil
# ^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `Root::Nested` but defines a constant that does not match this namespace

module Root::Nested

  ALLOWED_CONST = T.let(1, Integer)
  A::B = 2

  class Inner
    CONST = T.let(1, Integer)

    sig {returns(T::Boolean)}
    def method
      true
    end
  end
end

module Root
  module Nested
    class SomeClass
      class Deeper; end
    end

    class OtherClass
      class Deep2; end
    end
  end
end

  class Root::Stringy < String
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Class or method behavior may not be defined outside of the enclosing package namespace `Root::Nested`
end

class Root::Nested::Stringy < String
end

module Root
  extend T::Sig
# ^^^^^^^^^^^^^ error: Class or method behavior may not be defined outside of the enclosing package namespace `Root::Nested`
  NOT_IN_PACKAGE = T.let(1, Integer)
# ^^^^^^^^^^^^^^ error: File belongs to package `Root::Nested` but defines a constant that does not match this namespace

  sig {returns(NilClass)}
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Class or method behavior may not be defined outside of the enclosing package namespace `Root::Nested`
  def self.method
# ^^^^^^^^^^^^^^^ error: Class or method behavior may not be defined outside of the enclosing package namespace `Root::Nested`
    nil
  end
end


top_level_local = 2

sig {returns(String)}
def top_level_method
  'top'
end

module Root::ModNotInPackage
     # ^^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `Root::Nested` but defines a constant that does not match this namespace
end

class Root::ClassNotInPackage
    # ^^^^^^^^^^^^^^^^^^^^^^^ error: File belongs to package `Root::Nested` but defines a constant that does not match this namespace
end

module ::TopLevel
  class Foo
    sig {void}
    def foo
    end
  end
end
