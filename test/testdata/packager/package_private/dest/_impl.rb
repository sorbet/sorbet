# typed: strict

module Dest
  Source::Public
  Source::Private # error: `Source::Private` resolves but is declared `package_private` in `Source`

  Nested::Outer
  Nested::Outer::InnerPublic
  Nested::Outer::InnerPrivate # error: `Nested::Outer::InnerPrivate` resolves but is declared `package_private` in `Nested`

  Nested::PrivateOuter # error: `Source::Private` resolves but is declared `package_private` in `Source`
  Nested::PrivateOuter::InnerThing # error: `Source::Private` resolves but is declared `package_private` in `Source`
  Nested::PrivateOuter::InnerOtherThing # error: `Source::Private` resolves but is declared `package_private` in `Source`
end
