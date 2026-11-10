# typed: true
# spacer for exclude-from-file-update
module Root::B
  Root::A::Foo.new.foo
# ^^^^^^^^^^^^^^^^^^^^ error: Method `foo` on `Root::A::Foo` is package-private and cannot be called from package `Root::B`
  Root::A::Foo.bar
# ^^^^^^^^^^^^^^^^ error: Method `bar` on `T.class_of(Root::A::Foo)` is package-private and cannot be called from package `Root::B`
end
