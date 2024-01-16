# typed: true

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

  T.assert_type!(1, T.any(Integer, NilClass, String, Integer)) # exists to trigger a sanity check

  def generic_class_glb_bottom
    ret = T.cast(nil, T.all(String, T::Hash[String, String]))
    foo # error: This code is unreachable
  end

  def generic_class_glb_collapse
    ret = T.cast(nil, T.all(Object, T::Hash[String, String]))
    T.assert_type!(ret, T::Hash[String, String])

    T.assert_type!(0, T.all(Object, T::Hash[String, String])) # error: does not have asserted type `T::Hash[String, String]`
  end

  def generic_class_module_glb
    T.assert_type!(0, T.all(A, T::Hash[String, String])) # error: does not have asserted type `T.all(T::Hash[String, String], A)`
  end

  def generic_class_lub(b)
    if b == 1
      xs = T::Array[T.nilable(String)].new
    elsif b == 2
      xs = nil
    else
      xs = T::Array[T.untyped].new
    end
    
    # T.any(nil, T::Array[T.nilable(String)], T::Array[T.untyped]) => T.nilable(T::Array[T.untyped])
    T.reveal_type(xs) # error: Revealed type: `T.nilable(T::Array[T.untyped])`
  end

  def generic_class_nilable_array_tuple_lub
    a = T.let(nil, T.nilable(T::Array[Float]))
    b = T.let(nil, T.nilable(T::Array[Integer]))
    T.reveal_type([a, b]) # error: Revealed type: `[T.nilable(T::Array[Float]), T.nilable(T::Array[Integer])] (2-tuple)`
    # This reproduces a bug in Sorbet. In the buggy version, the above T.reveal_type passes,
    # but the below statement would not error.
    [a, b].each(&:lazy)
    #            ^^^^^ error: Method `lazy` does not exist on `NilClass`
  end

  module Boolean
  end
  T4 = T.type_alias{T.any(T.all(Boolean, T.nilable(FalseClass)), T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, T.nilable(FalseClass)), FalseClass, T.all(Boolean, NilClass), TrueClass)}

  def equivalent_and_types_lub
    T.reveal_type(T4) # error: Revealed type: `Runtime object representing type: T.any(T.all(Main::Boolean, T.nilable(FalseClass)), FalseClass, T.all(Main::Boolean, NilClass), TrueClass)`
  end

end
