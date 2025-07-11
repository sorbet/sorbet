# typed: strict
# enable-experimental-rbs-comments: true

class Foo
  #: (Integer, String) -> String
  def foo(x, y)
    y * x
  end
end

class Bar < Foo
  # @override
  #: (Integer, String) -> String
  def foo(x, y)
    super(
      ARGV.first, #: as String # error: Expected `Integer` but found `String` for argument `x
      ARGV.last #: as Integer # error: Expected `String` but found `Integer` for argument `y`
    ) #: as Integer # error: Expected `String` but found `Integer` for method result type
  end
end

class Baz < Foo
  # @override
  #: (Integer, String) -> String
  def foo(x, y)
    super(
      *[
        ARGV.first, #: as Integer
        ARGV.last #: as String
      ]
    )
  end
end

class Qux < Foo
  # @override
  #: (Integer, String) -> String
  def foo(x, y)
    super(ARGV.first, ARGV.last) #: as Integer
    #                                  ^^^^^^^ error: Expected `String` but found `Integer` for method result type
  end
end
