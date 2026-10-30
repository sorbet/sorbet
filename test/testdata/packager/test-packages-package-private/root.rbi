# typed: strict

class ::Module < Object
  sig do
    type_parameters(:U)
      .params(args: T.all(T.type_parameter(:U), T.any(Symbol, String)))
      .returns(T::Array[T.type_parameter(:U)])
  end
  private def package_private(*args); end # = args

  sig do
    type_parameters(:U)
      .params(args: T.all(T.type_parameter(:U), T.any(Symbol, String)))
      .returns(T::Array[T.type_parameter(:U)])
  end
  private def package_private_class_method(*args); end # = args
end
