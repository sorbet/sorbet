# typed: strict

class ::Module
  sig { params(name: T.nilable(Symbol)).void }
  private def package_private!(name = nil); end
end
