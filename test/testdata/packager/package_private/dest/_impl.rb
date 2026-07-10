# typed: strict

module Dest
  # TODO(gdritter): when we switch to public-by-default, the following error whould be removed
  Source::Public # error: `Source::Public` resolves but is not exported
  Source::Private # error: `Source::Private` resolves but is not exported
end
