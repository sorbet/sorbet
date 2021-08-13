# typed: true

module Main
  module Boolean
  end
  T4 = T.type_alias{T.any(T.all(Boolean, T.nilable(FalseClass)), T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, NilClass), T.all(Boolean, T.nilable(FalseClass)), FalseClass, T.all(Boolean, NilClass), TrueClass)}

  def equivalent_and_types_lub
    T.reveal_type(T4) # error: Revealed type: `<Type: T.any(T.all(Main::Boolean, T.nilable(FalseClass)), FalseClass, T.all(Main::Boolean, NilClass), TrueClass)>`
  end
  
end
