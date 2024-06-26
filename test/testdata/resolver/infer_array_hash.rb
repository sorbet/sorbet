# typed: strict

X0 = [1, '']
Y0 = [1, ''].freeze

X1 = [1, puts] # error: must have type annotations

X2 = {foo: 0, 'bar' => false} # error: must have type annotations
Y2 = {foo: 0, 'bar' => false}.freeze # error: must have type annotations

X3 = {foo: puts} # error: must have type annotations

X4 = [1, ['', 3]].freeze
Y4 = [1, ['', 3].freeze].freeze

X5 = [1, Object.new].freeze # error: must have type annotations
Y5 = [1, T.let(Object.new, Object)].freeze

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

X6 = [MyEnum::X, MyEnum::Y].freeze
Y6 = [Integer, String].freeze

class HasTypeMember
  extend T::Generic
  Elem = type_member
end

class HasTypeTemplate
  extend T::Generic
  Elem = type_template
end

X7 = [HasTypeMember].freeze
Y7 = [HasTypeTemplate].freeze

def revealed_types # error: does not have a `sig`
  T.reveal_type(X0) # error: `T::Array[T.any(Integer, String)]`
  T.reveal_type(Y0) # error: `[Integer, String] (2-tuple)`

  T.reveal_type(X1) # error: `T.untyped`

  T.reveal_type(X2) # error: `T.untyped`
  T.reveal_type(Y2) # error: `T.untyped`

  T.reveal_type(X3) # error: `T.untyped`

  T.reveal_type(X4) # error: `[Integer, T::Array[T.any(String, Integer)]] (2-tuple)`
  T.reveal_type(Y4) # error: `[Integer, [String, Integer]] (2-tuple)`

  T.reveal_type(X5) # error: `T.untyped`
  T.reveal_type(Y5) # error: `[Integer, Object] (2-tuple)`

  T.reveal_type(X6) # error: `[MyEnum::X, MyEnum::Y] (2-tuple)`
  T.reveal_type(Y6) # error: `[T.class_of(Integer), T.class_of(String)] (2-tuple)`

  T.reveal_type(X7) # error: `[T.class_of(HasTypeMember)] (1-tuple)`
  T.reveal_type(Y7) # error: `[T.class_of(HasTypeTemplate)[HasTypeTemplate, T.untyped]] (1-tuple)`
end
