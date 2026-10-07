# typed: true

module Second
# error: File belongs to package `First` but defines a constant that does not match this namespace
  class Foo # error: File belongs to package `First` but defines a constant that does not match this namespace
    def foo; end
  end
end
