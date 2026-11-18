# typed: true

module Root::B
  Root::A::Init.new(1)
# ^^^^^^^^^^^^^^^^^^^^ error: Method `initialize` on `Root::A::Init` is package-private and cannot be called from package `Root::B`

  Root::A::Gen[Integer].new(1)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Method `initialize` on `Root::A::Gen` is package-private and cannot be called from package `Root::B`
  Root::A::Gen.new(1)
# ^^^^^^^^^^^^^^^^^^^ error: Method `initialize` on `Root::A::Gen` is package-private and cannot be called from package `Root::B`

  # No error: `package_private_class_method :new` doesn't restrict `new`.
  Root::A::Wrapped.new(1)
end
