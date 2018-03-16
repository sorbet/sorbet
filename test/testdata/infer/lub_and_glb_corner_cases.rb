# @typed

module A
end
module B
end
module C
end
module D
end

module Main
  def cross_product
      ret = T.cast(nil, T.any(
          T.all(A, T.any(C, D)),
          T.all(B, T.any(C, D)),
      ))
      T.assert_type!(ret, T.all(T.any(A, B), T.any(C, D)))
  end

  def demorgans
      ret = T.cast(nil, T.all(
          T.any(A, B),
          T.any(B, C),
      ))
      T.assert_type!(ret, T.any(B, T.all(A, C)))
  end

  def any_of_all
      ret = T.cast(nil, T.any(
          T.any(A, T.all(B, C)),
          D,
      ))
      T.assert_type!(ret, T.any(A, T.all(B, C), D))
  end

  T.assert_type!(1, T.any(Integer, NilClass, String, Integer)) # exists to tigger a sanity check
end
