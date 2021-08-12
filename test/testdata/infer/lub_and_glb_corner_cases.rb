# typed: true

module Main
  module Boolean
  end
  # T4 = T.type_alias{T.any(T.all(Boolean, T.nilable(FalseClass)), T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, T.nilable(FalseClass)), FalseClass, T.all(Boolean, NilClass), TrueClass)}

  T5 = T.type_alias{T.any(T.all(Boolean, T.nilable(FalseClass)), T.all(Boolean, NilClass))}
  T6 = T.type_alias{T.any(T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, T.nilable(FalseClass)), FalseClass, T.all(Boolean, NilClass), TrueClass)}
  T7 = T.type_alias{T.any(T5, T6)}

  def equivalent_and_types_lub
    # T.reveal_type(T4)
    T.reveal_type(T7) # error: Revealed type: `<Type: T.any(FalseClass, TrueClass, T.all(Main::Boolean, T.nilable(FalseClass)), T.all(Main::Boolean, NilClass))>`
  end
  
end
