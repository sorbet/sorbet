# typed: strict

class ::Module
  sig do
    type_parameters(:U)
      .params(args: T.all(T.type_parameter(:U), T.any(Symbol, String)))
      .returns(T::Array[T.type_parameter(:U)])
  end
  private def package_private(*args); end

  sig do
    type_parameters(:U)
      .params(args: T.all(T.type_parameter(:U), T.any(Symbol, String)))
      .returns(T::Array[T.type_parameter(:U)])
  end
  private def package_private_class_method(*args); end
end
