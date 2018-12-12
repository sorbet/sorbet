# typed: true

class Generics
  extend T::Sig

  sig do
    params(
      cond: T.untyped,
      arr1: T::Array[String],
      arr2: T::Array[String],
    ).returns(T::Array[String])
  end
  def buildGLB(cond, arr1, arr2)
  T.assert_type!(if (cond == arr1)
                 if (cond == arr2)
                   cond
                 else
                   arr2
                 end
               else
                 arr1
               end,
            T::Array[String])
  end
end
