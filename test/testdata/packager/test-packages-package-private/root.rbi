# typed: strict

class ::Module < Object

  sig { params( arg0: T.any(Symbol, String)).returns(Symbol) }
  def package_private(*arg0); end

  sig { params(arg0: T.any(Symbol, String)).returns(T.self_type) }
  def package_private_class_method(*arg0); end

end
