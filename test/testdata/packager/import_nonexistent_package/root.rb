# typed: strict

module Root
  Root::A::Foo
# ^^^^^^^ error: `Root::A` is not imported
end
