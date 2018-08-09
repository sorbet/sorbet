# typed: strict

class Generics
  extend T::Helpers

  sig(
    cond: T.untyped,
    arr1: T::Array[String],
    arr2: T::Array[String],
  ).returns(T::Array[String])
  def buildLub(cond, arr1, arr2)
  T.assert_type!(if (cond)
                   arr1
                 else
                   arr2
                 end,
             T::Array[String])
  end
end
