# typed: true

module Root::A::Test
  # This package imports `Root::A` with `uses_internals: true`.
  Root::A::Outer::Inner
  Root::A::Public::X
  Root::A::SECRET
end
