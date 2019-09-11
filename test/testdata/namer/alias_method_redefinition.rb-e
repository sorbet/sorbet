# typed: true

# This used to crash us because we were using findMethod when we meant to use
# findMethodNoDealias when name mangling a symbol.
# It manifested as symbol.data(ctx)->name != origName.

class A
  class << self
    def foo(opts={}, *rest)
    end
    alias_method :__foo__, :foo # error: Redefining the existing method `A.__foo__` as a method alias
  end
end

class A
  def self.__foo__(*_); end
end


class B
  def self.__foo__(*_); end
end

class B
  class << self
    def foo(opts={}, *rest)
    end
    alias_method :__foo__, :foo # error: Redefining the existing method `B.__foo__` as a method alias
  end
end

class C
  def foo1; end
  def foo2; end
  alias_method :bar, :foo1
  alias_method :bar, :foo2 # error: Redefining method alias `C#bar` from `C#foo1` to `C#foo2`
end

# This one is kind of strange from a user's perspective.  Sorbet sees the
# alias_method after the second `def bar` because it doesn't see alias_method
# until resolver.
class D
  def foo; end
  alias_method :bar, :foo # error: Redefining the existing method `D#bar` as a method alias
  def bar(x); end
end
