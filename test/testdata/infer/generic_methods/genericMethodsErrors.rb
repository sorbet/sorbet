# typed: true
class Foo
  extend T::Generic
  extend T::Sig

  sig {type_parameters(:A, :A).params(a: T.type_parameter(:A)).returns(T.type_parameter(:A))}
  #                        ^^ error: Malformed `sig`: Type argument `A` was specified twice
  def id0(a)
    a
  end

  sig {type_parameters(:A).params(a: T.type_parameter(:B)).returns(T.type_parameter(:A))}  # error: Unspecified type parameter
  def id1(a)
    a
  end

  sig {type_parameters(:A).params(a: Integer).returns(Integer)}
  def id2(a)
    a
  end

  sig do
    params(
        blk: T.proc.params(arg0: Integer).type_parameters(:A).
        #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: can only be specified in outer sig
        #                                 ^^^^^^^^^^^^^^^ error: Call to method `type_parameters` on `T.proc.params(arg0: Integer).returns(<todo sym>)` mistakes a type for a value
          returns(T.type_parameter(:A)), # error: Unspecified type parameter
    )
    .returns(T::Array[T.type_parameter(:A)]) # error: Unspecified type parameter
  end
  def map(&blk);
    [blk.call(1)]
  end
end

class Bad
  extend T::Sig

  X = T.let(nil, T.nilable(T.type_parameter(:U)))
  #                                         ^^ error: Unspecified type parameter

  @x = T.let(nil, T.nilable(T.type_parameter(:U)))
  #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type for `@x`
  #                                          ^^ error: Unspecified type parameter

  sig {type_parameters(:U).params(y: T.type_parameter(:U)).void}
  def initialize(y)
    @y1 = y

    @y2 = T.let(y, T.type_parameter(:U))
    #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type for `@y2`
    #                               ^^ error: Unspecified type parameter
  end

  def foo
    T.reveal_type(@y1) # error: `T.untyped`
    T.reveal_type(@y2) # error: `T.untyped`

    @z = T.let(nil, T.nilable(T.type_parameter(:U)))
    #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type for `@z`
    #                                          ^^ error: Unspecified type parameter
    #                         ^^^^^^^^^^^^^^^^^^^^ error: Method `Bad#foo` does not declare any type parameters
  end
end

class BadGeneric
  extend T::Generic

  Elem = type_member {{upper: T.type_parameter(:U)}}
  #                                            ^^ error: Unspecified type parameter
end
