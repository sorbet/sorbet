# typed: true

module Root::B
  Root::A::Outer
# ^^^^^^^^^^^^^^ error: `Root::A::Outer` is `package_private!` in package `Root::A`

  # A constant nested inside a package-private namespace.
  Root::A::Outer::Inner
# ^^^^^^^^^^^^^^ error: `Root::A::Outer` is `package_private!` in package `Root::A`

  Root::A::SECRET
# ^^^^^^^^^^^^^^^ error: `Root::A::SECRET` is `package_private!` in package `Root::A`

  Root::A::Public
  Root::A::Public::Y
  Root::A::Public::X
# ^^^^^^^^^^^^^^^^^^ error: `Root::A::Public::X` is `package_private!` in package `Root::A`

  # Through a class alias.
  Root::A::PublicAlias
  Root::A::PublicAlias::Inner

  Root::A::PrivateAlias
# ^^^^^^^^^^^^^^^^^^^^^ error: `Root::A::PrivateAlias` is `package_private!` in package `Root::A`

  Root::A::MyEnum::Open
  Root::A::MyEnum::Secret
# ^^^^^^^^^^^^^^^^^^^^^^^ error: `Root::A::MyEnum::Secret` is `package_private!` in package `Root::A`

  x = T.let(1, Root::A::Public::TA)
  #            ^^^^^^^^^^^^^^^^^^^ error: `Root::A::Public::TA` is `package_private!` in package `Root::A`
  T.reveal_type(x) # error: Revealed type: `Root::A::Public::TA (unresolved)`

  class IncludesMixins
    include Root::A::PublicMixin

    # Resolves through an ancestor, without naming any package-private component.
    Hidden
  # ^^^^^^ error: `Root::A::PublicMixin::Hidden` is `package_private!` in package `Root::A`

    include Root::A::PrivateMixin
    #       ^^^^^^^^^^^^^^^^^^^^^ error: `Root::A::PrivateMixin` is `package_private!` in package `Root::A`
  end

  class IncludesReExports
    include Root::A::ReExports

    # Resolves through an ancestor that's covered by a marker on its enclosing namespace.
    Thing
  # ^^^^^ error: `Root::A::Outer::Mod::Thing` is package-private, because `Root::A::Outer` is `package_private!` in package `Root::A`
  end
end
