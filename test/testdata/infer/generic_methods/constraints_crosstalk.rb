# typed: true
class Test
  extend T::Sig

  sig {params(value: T.any(A, B)).returns(NilClass)}
  private def serialize_value(value)
      # this is a horrible example. We hate to solve constraint that comes from multiple methods at once.
      # As far as I can tell, this error is actually correct: given the blk types,
      # it's not possible for either `A#map` or `B#map` to call their block,
      # because neither one of them has a way to produce a value of type `T.type_parameter(:U)` to the block.
      T.let(value.map {|val| 1}, T.any(T::Array[Integer], Integer))
      #                      ^ error: This code is unreachable
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
