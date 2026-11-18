# typed: true

module Root::A
  class Init
    package_private def initialize(x); end
  end

  class Gen
    extend T::Generic
    E = type_member(:out)

    package_private def initialize(x); end
  end

  # `new` is inherited from `Class`, not defined on `Wrapped`, so this doesn't
  # make `Wrapped.new` package-private.
  class Wrapped
    package_private_class_method :new

    def initialize(x); end
  end

  # Calls from the same package.
  Init.new(1)
  Gen[Integer].new(1)
  Gen.new(1)
  Wrapped.new(1)
end
