# typed: strict

module Dest
  Source::Public
  Source::Private # error: `Source::Private` resolves but is declared `package_private` in `Source`
end
