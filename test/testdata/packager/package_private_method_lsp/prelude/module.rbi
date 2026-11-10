# typed: strict

class ::Module
  sig { params(args: Symbol).void }
  private def package_private(*args); end

  sig { params(args: Symbol).void }
  private def package_private_class_method(*args); end
end
