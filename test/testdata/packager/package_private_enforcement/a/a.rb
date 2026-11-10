# typed: strict

module Root::A
  SECRET = 1
  package_private!(:SECRET)

  class Outer
    package_private!

    class Inner; end
    CONST = 1

    module Mod
      class Thing; end
    end

    # Covered by the marker on `Outer`.
    InnerAlias = Inner
  end

  PublicAlias = Outer
# ^^^^^^^^^^^ error: `Root::A::PublicAlias` is a public alias of package-private `Root::A::Outer`

  PrivateAlias = Outer
  package_private!(:PrivateAlias)

  PrivateConstantAlias = Outer
# ^^^^^^^^^^^^^^^^^^^^ error: `Root::A::PrivateConstantAlias` is a public alias of package-private `Root::A::Outer`
  private_constant :PrivateConstantAlias

  # Includes a module covered by the marker on `Outer`.
  module ReExports
    include Outer::Mod
  end

  class Public
    X = 1
    package_private!(:X)

    Y = 1

    TA = T.type_alias { Integer }
    package_private!(:TA)
  end

  class MyEnum < T::Enum
    enums do
      Secret = new
      Open = new
    end
    package_private!(:Secret)
  end

  module PublicMixin
    class Hidden
      package_private!
    end
  end

  module PrivateMixin
    package_private!
  end

  # References from the same package.
  Outer::Inner
  Public::X
  SECRET
end
