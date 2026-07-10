# typed: strict

module Dest
  Source::Public
  Source::Private # error: `Source::Private` resolves but is declared `package_private` in `Source`

  Nested::Outer::InnerPublic
  Nested::Outer::InnerPrivate # error: `Nested::Outer::InnerPrivate` resolves but is declared `package_private` in `Nested`
end
