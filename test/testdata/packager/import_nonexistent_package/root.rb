# typed: strict

module Root::Probe
  Root::A::Foo
# ^^^^^^^ error: `Root::A` is not imported
end
