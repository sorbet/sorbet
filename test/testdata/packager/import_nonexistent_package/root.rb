# typed: strict

module Root
  Root::A::Foo
# ^^^^^^^^^^^^ error: `Root::A::Foo` resolves but its package is not imported
end
