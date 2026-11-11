# typed: true

# Without --sorbet-packages, `package_private!` markers are ignored.

class Module
  private def package_private!(name = nil); end
end

module A
  package_private!

  class B
    package_private!

    X = 1
    package_private!(:X)
  end
end

A::B::X
