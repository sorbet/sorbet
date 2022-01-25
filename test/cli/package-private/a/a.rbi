# typed: strict

class ::Module < Object
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def package_private(*arg0); end

  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def package_private_class_method(*arg0); end
end
