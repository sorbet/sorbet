# typed: true

module A
  Result = T.type_alias {B::Result[Integer]}
end

module B
  Result = C::Result
end

module C
  module Result
    extend T::Generic

    X = type_member(:out)
  end
end
