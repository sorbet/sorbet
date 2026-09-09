# typed: true

module Second::Other
     # ^^^^^^ error: File belongs to package `First` but defines a constant that does not match this namespace
  class Foo
  end
end
