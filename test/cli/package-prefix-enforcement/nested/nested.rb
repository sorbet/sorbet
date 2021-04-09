# typed: strict

extend T::Sig

module Wrong
  class Inside; end
end

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
    class SomeClass; end
  end
end

module Root
  extend T::Sig
  NOT_IN_PACKAGE = T.let(1, Integer)

  sig {returns(NilClass)}
  def self.method
    nil
  end
end


top_level_local = 2

sig {returns(String)}
def top_level_method
  'top'
end

module Root::ModNotInPackage
end

class Root::ClassNotInPackage
end

module ::TopLevel
  class Foo
    sig {void}
    def foo
    end
  end
end
