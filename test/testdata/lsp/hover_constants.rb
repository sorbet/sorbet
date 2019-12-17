# typed: true

# This file documents incorrect behavior when we findDocumentation for
# constants via hover.
#
# See: https://github.com/sorbet/sorbet/issues/2311
#
# Note: completion also uses this logic for showing documentation.
# See test/testdata/lsp/completion/constants_all_kinds.rb for a test to look at.

# Docs for AAA
class AAA; end # right
#     ^ hover: Docs for AAA

# Docs for BBB
class BBB # right
  #   ^ hover: Docs for BBB
  extend T::Generic

  # Docs for XXX
  XXX = nil # wrong
  # ^ hover: Docs for XXX

  # Docs for CCC
  CCC = AAA # wrong? arguably showing the dealiased docs is fine
  # ^ hover: Docs for AAA

  # Docs for EEE
  EEE = type_member
  # ^ hover: Docs for EEE
end
