# typed: strict

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

  def generic_class_glb_bottom
    ret = T.cast(nil, T.all(String, T::Hash[String, String]))
    T.assert_type!(ret, T.noreturn)
  end

  def generic_class_glb_collapse
    ret = T.cast(nil, T.all(Object, T::Hash[String, String]))
    T.assert_type!(ret, T::Hash[String, String])

    T.assert_type!(0, T.all(Object, T::Hash[String, String])) # error: does not have asserted type `T::Hash[String, String]`
  end

  def generic_class_module_glb
    T.assert_type!(0, T.all(A, T::Hash[String, String])) # error: does not have asserted type `T.all(T::Hash[String, String], A)`
  end
end
