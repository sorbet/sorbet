# typed: strict

module MyPackage
  class HasSingletonIvar
    @outside = T.let(0, Integer)

    class Nested
      @nested = T.let(0, Integer)
    end
  end
end
