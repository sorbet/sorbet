# typed: true

# This used to crash us because we were using findMethod when we meant to use
# findMethodNoDealias when name mangling a symbol.
# It manifested as symbol.data(ctx)->name != origName.

class A
  class << self
    def foo(opts={}, *rest)
    end
    alias_method :__foo__, :foo
  end
end

class A
  def self.__foo__(*_); end # error: Redefining method alias `A.__foo__`
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
