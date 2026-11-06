# typed: strict

class ::Module
  sig { params(name: T.nilable(Symbol)).void }
  private def package_private!(name = nil); end

  sig { params(args: T.any(Symbol, String)).returns(T.untyped) }
  private def package_private(*args); end

  sig { params(args: T.any(Symbol, String)).returns(T.untyped) }
  private def package_private_class_method(*args); end
end
