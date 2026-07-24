# typed: strict

module Dest
  Source::Public
  Source::Private # error: `Source::Private` resolves but is declared `package_private` in `Source`

  Nested::Outer
  Nested::Outer::InnerPublic
  Nested::Outer::InnerPrivate # error: `Nested::Outer::InnerPrivate` resolves but is declared `package_private` in `Nested`

  Nested::PrivateOuter # error: `Nested::PrivateOuter` resolves but is declared `package_private` in `Nested`
  Nested::PrivateOuter::InnerThing # error: `Nested::PrivateOuter::InnerThing` resolves but is declared `package_private` in `Nested`
  Nested::PrivateOuter::InnerOtherThing # error: `Nested::PrivateOuter::InnerOtherThing` resolves but is declared `package_private` in `Nested`
end
