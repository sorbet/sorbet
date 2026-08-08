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

class SuperBlockParent
  #: (*untyped) { (untyped) -> untyped } -> untyped
  def forwarding_super(*args); end

  #: (Symbol) { (untyped) -> untyped } -> untyped
  def explicit_super(arg); end
end

class SuperBlockChild < SuperBlockParent
  # @override
  #: (*untyped) { (untyped) -> untyped } -> untyped
  def forwarding_super(*args)
    super do |local_record|
      return_value = nil #: Hash[Symbol, untyped]?
      T.reveal_type(return_value) # error: Revealed type: `T.nilable(T::Hash[Symbol, T.untyped])`
    end
  end

  # @override
  #: (Symbol) { (untyped) -> untyped } -> untyped
  def explicit_super(arg)
    super(arg) do |local_record|
      return_value = nil #: String?
      T.reveal_type(return_value) # error: Revealed type: `T.nilable(String)`
    end
  end
end
