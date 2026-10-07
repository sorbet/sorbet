# typed: strict

class Foo # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
  class FromUnqualified # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
  end
end

class ::Foo
  class FromExplicitRoot
  end
end
