# typed: true

extend T::Sig

class A < T:Struct
  prop :foo, T.nilable(T.untyped)
end

class B < T::ImmutableStruct
  prop :foo, Integer
end
