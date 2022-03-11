# typed: true
extend T::Sig

sig do
  type_parameters(:T)
  .params(x: T.nilable(T.type_parameter(:T)))
  .returns(T.type_parameter(:T))
end
def refute_nil(x)
  T.reveal_type(x) # error: `T.nilable(T.type_parameter(:T) (of Object#refute_nil))`
  case x
  when NilClass then raise "was not nil"
  else
    T.reveal_type(x) # error: `T.type_parameter(:T) (of Object#refute_nil)`
    x
  end
end

nilable_integer = T.let(0, T.nilable(Integer))
integer_or_nilclass = T.let(0, T.any(Integer, NilClass))
untyped = T.unsafe(0)

x = refute_nil(nilable_integer)
T.reveal_type(x) # error: `Integer`

x = refute_nil(integer_or_nilclass)
T.reveal_type(x) # error: `Integer`

x = refute_nil(untyped)
T.reveal_type(x) # error: `T.untyped`


sig do
  type_parameters(:T)
  # Even if we write the parameters this way, they'll currently get re-ordered.
  # This happens because we sort the arguments to `lub` based on the value of
  # `TypePtr::kind` to cut the implementation in half.
  #
  # If that ever stopped happening, we'd likely have to change
  # isSubTypeUnderConstraint to be able to pass the tests below, or decide that
  # we can't support the behavior below anymore.
  .params(x: T.any(T.type_parameter(:T), NilClass))
  .returns(T.type_parameter(:T))
end
def refute_nil_opposite_order(x)
  T.reveal_type(x) # error: `T.nilable(T.type_parameter(:T) (of Object#refute_nil_opposite_order))`
  case x
  when NilClass then raise "was not nil"
  else
    T.reveal_type(x) # error: `T.type_parameter(:T) (of Object#refute_nil_opposite_order)`
    x
  end
end

x = refute_nil_opposite_order(nilable_integer)
T.reveal_type(x) # error: `Integer`

x = refute_nil_opposite_order(integer_or_nilclass)
T.reveal_type(x) # error: `Integer`

x = refute_nil_opposite_order(untyped)
T.reveal_type(x) # error: `T.untyped`
