# typed: true
class Foo
  extend T::Generic
  extend T::Sig

  sig {type_parameters(:A, :A).params(a: T.type_parameter(:A)).returns(T.type_parameter(:A))}  # error: Malformed signature
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
        blk: T.proc.params(arg0: Integer).type_parameters(:A). # error: can only be specified in outer sig
          returns(T.type_parameter(:A)), # error: Unspecified type parameter
    )
    .returns(T::Array[T.type_parameter(:A)]) # error: Unspecified type parameter
  end
  def map(&blk);
    [blk.call(1)]
  end
end
