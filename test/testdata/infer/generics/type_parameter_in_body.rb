# typed: true
extend T::Sig

sig do
  type_parameters(:ExistsOnOtherMethod)
    .params(x: T.type_parameter(:ExistsOnOtherMethod))
    .void
end
def example_method_for_coverage(x)
  # This ensures that there is at least some <T ExistsOnOtherMethod>$1> name in
  # the symbol table, just on a different method
end

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .returns(T::Array[T.type_parameter(:U)])
end
def example(x)
  xs = T::Array[T.type_parameter(:U)].new
  T.reveal_type(xs) # error: `T::Array[T.type_parameter(:U) (of Object#example)]`
  xs << x
  xs << "nope"
  #     ^^^^^^ error: Expected `T.type_parameter(:U) (of Object#example)` but found `String("nope")`

  0.times do
    T.type_parameter(:U).does_not_exist
    #                    ^^^^^^^^^^^^^^ error: Call to method `does_not_exist` on `T.type_parameter(:U) (of Object#example)` mistakes a type for a value
  end

  T.type_parameter(:DoesNotExist)
  #                ^^^^^^^^^^^^^ error: Type parameter `:DoesNotExist` does not exist on `Object#example`
  T.type_parameter(:ExistsOnOtherMethod)
  #                ^^^^^^^^^^^^^^^^^^^^ error: Type parameter `:ExistsOnOtherMethod` does not exist on `Object#example`

  xs
end

sig {void}
def not_generic
  T.type_parameter(:U) # error: Method `Object#not_generic` does not declare any type parameters
end

# Current heuristic for reporting this error is not as precise as it could be.
# We may want to restrict this in the future, but this is such an uncommon case
# that it seems maybe not worthwhile.
T.type_parameter(:U)
class ClassTopLevel
  T.type_parameter(:U)
end
