# typed: strict
class Foo
  extend T::Helpers

  type_parameters(:A, :A).sig(a: T.type_parameter(:A)).returns(T.type_parameter(:A))  # error: Malformed signature
  def id0(a)
    a
  end

  type_parameters(:A).sig(a: T.type_parameter(:B)).returns(T.type_parameter(:A))  # error: Unspecified type parameter
  def id1(a)
    a
  end

  type_parameters(:A).sig(a: Integer).returns(Integer)
  def id2(a)
    a
  end

  sig(
      blk: T.proc(arg0: Integer).type_parameters(:A). # error: can only be specified in outer sig
        returns(T.type_parameter(:A)), # error: Unspecified type parameter
  )
  .returns(T::Array[T.type_parameter(:A)]) # error: Unspecified type parameter
  def map(&blk);
    [blk.call(1)]
  end
end
