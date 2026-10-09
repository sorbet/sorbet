# typed: true
module CSV::DEFAULT_OPTIONS::PayloadLspModule
#      ^^^^^^^^^^^^^^^^^^^^ error: Redefining constant `DEFAULT_OPTIONS` as a class or module
  def foo
    1
  end
end
class Array::Elem::PayloadLspClass
#     ^^^^^^^^^^^ error: Redefining constant `Elem` as a class or module
  def foo
    1
  end
end
T.assert_type!(CSV::DEFAULT_OPTIONS, T::Hash[T.untyped, T.untyped])
T.assert_type!([1], T::Array[Integer])
