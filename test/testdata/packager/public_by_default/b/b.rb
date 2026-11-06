# typed: true

module Root::B
  Root::A::Exported
  Root::A::Unexported
# ^^^^^^^^^^^^^^^^^^^ error: `Root::A::Unexported` resolves but is not exported from `Root::A`
  Root::A::UNEXPORTED_CONST
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Root::A::UNEXPORTED_CONST` resolves but is not exported from `Root::A`
  Root::A::Namespace::Nested
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Root::A::Namespace::Nested` resolves but is not exported from `Root::A`
  Root::C::Unexported

  # A constant from a package that isn't imported.
  Root::D::Thing
# ^^^^^^^ error: `Root::D` is not imported
end
