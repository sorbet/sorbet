# typed: true

module Root::B
  Root::A::Outer

  # A constant nested inside a package-private namespace.
  Root::A::Outer::Inner

  Root::A::SECRET

  Root::A::Public
  Root::A::Public::Y
  Root::A::Public::X

  # Through a class alias.
  Root::A::PublicAlias
  Root::A::PublicAlias::Inner

  Root::A::PrivateAlias

  Root::A::MyEnum::Open
  Root::A::MyEnum::Secret

  x = T.let(1, Root::A::Public::TA)
  T.reveal_type(x) # error: Revealed type: `Integer`

  class IncludesMixins
    include Root::A::PublicMixin

    # Resolves through an ancestor, without naming any package-private component.
    Hidden

    include Root::A::PrivateMixin
  end

  class IncludesReExports
    include Root::A::ReExports

    # Resolves through an ancestor that's covered by a marker on its enclosing namespace.
    Thing
  end
end
