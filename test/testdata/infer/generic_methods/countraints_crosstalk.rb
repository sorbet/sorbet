# typed: true
class Test
  extend T::Sig

  sig {params(value: T.any(A, B)).returns(NilClass)}
  private def serialize_value(value)
      # this is a horrible example. We hate to solve constraint that comes from multiple methods at once.
      #
      T.let(value.map {|val| 1}, T.any(T::Array[Integer], Integer))
      nil
  end
end

class A
  extend T::Generic
  extend T::Sig

  sig do
    type_parameters(:U).params(
          blk: T.proc.params(arg0: T.type_parameter(:U)).returns(T.type_parameter(:U)),
    )
    .returns(T::Array[T.type_parameter(:U)])
  end
  def map(&blk)
    T.unsafe(nil)
  end
end

class B
  extend T::Generic
  extend T::Sig

  sig do
    type_parameters(:U).params(
          blk: T.proc.params(arg0: T.type_parameter(:U)).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def map(&blk)
    T.unsafe(nil)
  end
end
