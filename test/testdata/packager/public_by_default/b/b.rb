# typed: true

module Root::B
  Root::A::Exported
  Root::A::Unexported
  Root::A::UNEXPORTED_CONST
  Root::A::Namespace::Nested
  Root::C::Unexported

  # A constant from a package that isn't imported.
  Root::D::Thing
# ^^^^^^^ error: `Root::D` is not imported
end
